/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 08:39:33 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 10:46:07 by mpellegr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <poll.h>
#include <unistd.h>

#include <stdexcept>
#include <vector>
#include <csignal>
#include <iostream>

#include "Logger.hpp"
#include "netinet/in.h"
#include "sys/socket.h"

Server::Server() : Server("default", 8123)
{
}

Server::Server(std::string server_pass, int server_port)
	: server_pass_{server_pass}, server_port_{server_port}
{
	_instance = this;
}

Server::Server(const Server& o) : Server(o.server_pass_, o.server_port_)
{
}

Server& Server::operator=(const Server& o)
{
	if (this == &o)
	{
		return (*this);
	}
	this->server_port_ = o.server_port_;
	this->server_pass_ = o.server_pass_;
	return (*this);
}

Server *Server::_instance = nullptr;

void Server::handleSignal(int signum) {
	static_cast<void>(signum);
	std::cout << "\nServer shutting down...\n";
	if (_instance)
		close(_instance->_serverSocket);
	exit(0);
}

void Server::startServer()
{
	struct sockaddr_in serverAddr;

	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1) throw std::runtime_error("failed to create socket");

	// setNonBlocking(serverSocket);

	// Enable port reuse
	int opt = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(this->server_port_);

	if (bind(_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		throw std::runtime_error("failed to bind socket");

	if (listen(_serverSocket, 5) == -1)
		throw std::runtime_error("listen() failed");
	Logger::log(Logger::INFO, "Server listening on port " +
	                              std::to_string(this->server_port_));

	std::vector<struct pollfd> pollFds;
	pollFds.push_back({_serverSocket, POLLIN, 0});

	signal(SIGINT, handleSignal); // 

	while (1)
	{
		int eventCount = poll(pollFds.data(), pollFds.size(), -1);
		if (eventCount == -1)
		{
			throw std::runtime_error("Error: poll");
		}

		for (size_t i = 0; i < pollFds.size(); i++)
		{
			if (pollFds[i].revents & POLLIN)
			{
				if (pollFds[i].fd == _serverSocket)
				{  // new client trying to connect
					struct sockaddr_in clientAddr;
					socklen_t clientLen = sizeof(clientAddr);
					int clientSocket = accept(
						_serverSocket, (sockaddr*)&serverAddr, &clientLen);
					if (clientSocket > 0)
					{
						Logger::log(Logger::INFO,
						            "New client connected: " +
						                std::to_string(clientSocket));
						pollFds.push_back({clientSocket, POLLIN, 0});
						this->users_[clientSocket] = User(clientSocket);
					}
				}
				else
				{
					// Receive data from client
					std::string buf(1024, 0);
					if (!users_[pollFds[i].fd].receiveData(buf))
					{
						std::string msg = "Client " +
						                  std::to_string(pollFds[i].fd) +
						                  " disconnected";
						Logger::log(Logger::INFO, msg);

						pollFds[i].fd = -1;  // Ignore this in the future,
						                     // delete before next iteration?
						continue;
					}
					Logger::log(Logger::DEBUG,
					            "Client " + std::to_string(pollFds[i].fd) +
					                " sent: " + buf);

					// TODO: Do something with the client data
					parseMessage(buf);
				}
			}
		}
	}
	// close(_serverSocket);  // Never reaching, handle somehow
}

void Server::parseMessage(std::string& msg)
{
	COMMANDTYPE cmd = getMessageType(msg);
	if (cmd == NONE)
	{
		Logger::log(Logger::ERROR, "Invalid command: " + msg);
	}
}

COMMANDTYPE Server::getMessageType(std::string& msg)
{
	if (msg.compare(0, 5, "PASS ") == 0) return (PASS);
	if (msg.compare(0, 5, "NICK ") == 0) return (NICK);
	if (msg.compare(0, 5, "USER ") == 0) return (USER);
	return (NONE);
}
