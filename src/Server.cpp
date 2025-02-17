/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 08:39:33 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 18:07:21 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "Logger.hpp"
#include "Message.hpp"

Server::Server() : Server("default", 8123)
{
}

Server::Server(std::string server_pass, int server_port)
    : server_pass_{server_pass}, server_port_{server_port}
{
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

void Server::startServer()
{
	int serverSocket;
	struct sockaddr_in serverAddr;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1) throw std::runtime_error("failed to create socket");

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(this->server_port_);

	int opt{1};
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	               sizeof(opt)))
	{
		throw std::runtime_error("setsockopt");
	}

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		throw std::runtime_error("failed to bind socket");

	if (listen(serverSocket, 5) == -1)
		throw std::runtime_error("listen() failed");
	Logger::log(Logger::INFO, "Server listening on port " +
	                              std::to_string(this->server_port_));

	std::vector<struct pollfd> pollFds;
	pollFds.push_back({serverSocket, POLLIN, 0});

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
				if (pollFds[i].fd == serverSocket)
				{  // new client trying to connect
					struct sockaddr_in clientAddr;
					socklen_t clientLen = sizeof(clientAddr);
					int clientSocket = accept(
					    serverSocket, (sockaddr*)&serverAddr, &clientLen);
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

					Message msg{buf};
					try
					{
						msg.parseMessage();
						executeCommand(msg, users_[pollFds[i].fd]);
					}
					catch (std::invalid_argument& e)
					{
						Logger::log(Logger::WARNING, e.what());
					}
				}
			}
		}
	}
	close(serverSocket);  // Never reaching, handle somehow
}

void Server::executeCommand(Message& msg, User& usr)
{
	if (msg.getType() == PASS)
	{
		executePassCommand(msg, usr);
	}
	else if (msg.getType() == USER)
	{
		executeUserCommand(msg, usr);
	}
	else if (msg.getType() == NICK)
	{
		executeNickCommand(msg, usr);
	}
}

void Server::executePassCommand(Message& msg, User& usr)
{
	if (msg.getArgs().size() != 1)
	{
		throw std::invalid_argument{"Invalid number of arguments"};
	}
	usr.setPassword(msg.getArgs()[0]);
	Logger::log(Logger::DEBUG, "Set user password to " + msg.getArgs()[0]);
}

void Server::executeNickCommand(Message& msg, User& usr)
{
	if (msg.getArgs().size() != 1)
	{
		throw std::invalid_argument{"Invalid number of arguments"};
	}
	usr.setNick(msg.getArgs()[0]);
	if (!usr.isRegistered())
	{
		attemptRegistration(usr);
	}
}

void Server::executeUserCommand(Message& msg, User& usr)
{
	if (msg.getArgs().size() != 4)
	{
		throw std::invalid_argument{"Invalid number of arguments"};
	}
	usr.setUsername(msg.getArgs().front());
	if (!usr.isRegistered())
	{
		attemptRegistration(usr);
	}
}

void Server::attemptRegistration(User& usr)
{
	if (usr.getPassword().size() == 0 || usr.getNick().size() == 0 ||
	    usr.getUsername().size() == 0)
	{
		return;
	}
	if (usr.getPassword() == server_pass_)
	{
		usr.registerUser();
	}
	else
	{
		throw std::invalid_argument{"Invalid password"};
	}
}
