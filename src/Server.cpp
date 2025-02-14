/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 08:39:33 by pleander          #+#    #+#             */
/*   Updated: 2025/02/14 13:27:01 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <poll.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <vector>

#include "netinet/in.h"
#include "sys/socket.h"

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

	// setNonBlocking(serverSocket);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(this->server_port_);

	if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		throw std::runtime_error("failed to bind socket");

	if (listen(serverSocket, 5) == -1)
		throw std::runtime_error("listen() failed");
	std::cout << "Server listening on port " << this->server_port_ << "...\n";

	std::vector<struct pollfd> pollFds;
	pollFds.push_back({serverSocket, POLLIN, 0});

	while (1)
	{
		int eventCount = poll(pollFds.data(), pollFds.size(), -1);
		if (eventCount == -1)
		{
			std::cerr << "poll() failed\n";
			break;
		}

		for (size_t i = 0; i < pollFds.size(); i++)
		{
			if (pollFds[i].fd == serverSocket)
			{  // new client trying to connect
				struct sockaddr_in clientAddr;
				socklen_t clientLen = sizeof(clientAddr);
				int clientSocket =
				    accept(serverSocket, (sockaddr*)&serverAddr, &clientLen);
				if (clientSocket > 0)
				{
					std::cout << "New client connected: " << clientSocket
					          << "\n";
					//					setNonBlocking(clientSocket);
					pollFds.push_back({clientSocket, POLLIN, 0});
					this->users_[clientSocket] = User(clientSocket);
				}
			}
			else
			{
				std::cout << "Client " << pollFds[i].fd << " is sending data."
				          << std::endl;

				// 	// client sending data
				// 	char buffer[1024];
				// 	memset(buffer, 0, sizeof(buffer));
				// 	int bytes = recv(pollFds[i].fd, buffer, sizeof(buffer), 0);
				// 	if (bytes > 0)
				// 	{
				// 		std::cout << "Client: " << buffer << "\n";
				// 		std::string message = ":server 001 Welcome to IRC!\n";
				// 		send(pollFds[i].fd, message.c_str(), message.size(), 0);
				// 	}
				// 	else if (bytes == 0)
				// 	{
				// 		std::cout << "client disconnected: " << pollFds[i].fd
				// 		          << std::endl;
				// 		close(pollFds[i].fd);
				// 	}
				// }
			}
		}
		close(serverSocket);
	}
}
