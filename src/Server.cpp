/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 08:39:33 by pleander          #+#    #+#             */
/*   Updated: 2025/02/18 07:30:36 by mpellegr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <csignal>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Channel.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "netinet/in.h"
#include "sys/socket.h"

Server::Server() : Server("default", 8123, "defaultServerName")
{
}

Server::Server(std::string server_pass, int server_port,
               std::string server_name)
    : server_pass_{server_pass},
      server_port_{server_port},
      server_name_{server_name}
{
	_server = this;
}

Server::Server(const Server& o)
    : Server(o.server_pass_, o.server_port_, o.server_name_)
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

Server* Server::_server = nullptr;

void Server::handleSignal(int signum)
{
	static_cast<void>(signum);
	std::cout << "\nServer shutting down...\n";
	if (_server) close(_server->_serverSocket);
	exit(0);
}
const std::map<COMMANDTYPE, Server::executeFunc> Server::execute_map_ = {
    {PASS, &Server::executePassCommand},
    {NICK, &Server::executeNickCommand},
    {USER, &Server::executeUserCommand},
    {OPER, &Server::executeOperCommand},
    {PRIVMSG, &Server::executePrivmsgCommand},
    {JOIN, &Server::executeJoinCommand},
    {PART, &Server::executePartCommand},
    {INVITE, &Server::executeInviteCommand},
    {WHO, &Server::executeWhoCommand},
    {QUIT, &Server::executeQuitCommand},
    {MODE, &Server::executeModeCommand},
    {KICK, &Server::executeKickCommand},
    {NOTICE, &Server::executeNoticeCommand},
    {TOPIC, &Server::executeTopicCommand},
    // Extend this list when we have more functions
};

void Server::startServer()
{
	struct sockaddr_in serverAddr;

	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error("failed to create socket");

	// setNonBlocking(serverSocket);

	// Enable port reuse
	int opt = 1;
	setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(this->server_port_);

	if (bind(_serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
		// int opt{1};
		// if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
		// &opt,
		//                sizeof(opt)))
		// {
		// 	throw std::runtime_error("setsockopt");
		// }

		// if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
		// -1)
		throw std::runtime_error("failed to bind socket");

	if (listen(_serverSocket, 5) == -1)
		throw std::runtime_error("listen() failed");
	Logger::log(Logger::INFO, "Server listening on port " +
	                              std::to_string(this->server_port_));

	std::vector<struct pollfd> pollFds;
	pollFds.push_back({_serverSocket, POLLIN, 0});

	signal(SIGINT, handleSignal);  //

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
					try
					{
						std::string buf;
						User* user = &(users_[pollFds[i].fd]);
						if (!user->receiveData())
						{
							std::string msg = "Client " +
							                  std::to_string(pollFds[i].fd) +
							                  " disconnected";
							Logger::log(Logger::INFO, msg);

							pollFds[i].fd =
							    -1;  // Ignore this in the future,
							         // delete before next iteration?
							continue;
						}
						while (user->getNextMessage(buf))
						{
							try
							{
								Message msg{buf};
								msg.parseMessage();
								executeCommand(msg, users_[pollFds[i].fd]);
							}
							catch (std::invalid_argument& e)
							{
								Logger::log(Logger::WARNING, e.what());
							}
						}
					}
					catch (std::invalid_argument& e)
					{
						Logger::log(Logger::WARNING, e.what());
					}
				}
			}
		}
	}
	// close(_serverSocket);  // Never reaching, handle somehow
}

std::map<std::string, Channel>& Server::getChannels()
{
	return _channels;
}
std::map<int, User>& Server::getUsers()
{
	return users_;
};

void joinChannel(std::vector<std::string> channels, int clientFd,
                 Server* _server)
{
	for (size_t i = 0; i < channels.size(); i++)
	{
		if (channels[i].empty() || channels[i][0] != '#')
		{
			std::cout << "Invalid channel name: " << channels[i] << std::endl;
			continue;
		}
		std::map<std::string, Channel>& existingChannels =
		    _server->getChannels();
		auto it = existingChannels.find(channels[i]);
		// std::cout << it->first << std::endl;
		if (it == existingChannels.end())
		{
			Channel newChannel(channels[i], clientFd);
			existingChannels.insert({newChannel.getName(), newChannel});
			Logger::log(Logger::INFO, "user " + std::to_string(clientFd) +
			                              " created new channel " +
			                              channels[i]);
			it = existingChannels.find(channels[i]);
			// std::cout << "here?\n";
		}
		else
			it->second.addUser(clientFd);
	}
}

void handleMessages(std::vector<std::string> messages, int clientFd,
                    Server* _server)
{
	std::string channel, message;  // possible to have multile messages?

	channel = messages[0];
	message = messages[1];
	if (!channel.empty() && !message.empty())
	{
		std::map<std::string, Channel>& existingChannels =
		    _server->getChannels();
		auto it = existingChannels.find(channel);
		if (it != existingChannels.end() &&
		    it->second.isUserInChannel(clientFd))
		{
			it->second.displayMessage(clientFd, message);
		}
		else
		{
			std::string msg = "User " + std::to_string(clientFd) +
			                  " is not in channel " + channel +
			                  " or channel doesn't exist.\n";
			Logger::log(Logger::ERROR, msg);
		}
	}
}

void Server::executeCommand(Message& msg, User& usr)
{
	// if (!usr.isRegistered() && msg.getType() > 4) // commented out just for
	// testing
	// {
	// 	return;
	// }
	std::map<COMMANDTYPE, executeFunc>::const_iterator it =
	    execute_map_.find(msg.getType());
	if (it != execute_map_.end())
	{
		(this->*(it->second))(msg, usr);
	}
	else
	{
		throw std::invalid_argument("Unsupported command");
	}
}

// this is actually not good. It's easier to hardcode all the different RPL &
// ERR msgs.
void Server::sendReply(User& usr, int numeric, const std::string& command,
                       const std::string& message)
{
	std::ostringstream oss;
	oss << ":" << server_name_ << " " << numeric << " " << usr.getNick() << " "
	    << command << " :" << message;
	std::string reply = oss.str();
	usr.sendData(reply);
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

// Maybe this bloat will get moved to individual .cpp files
void Server::executeOperCommand(Message& msg, User& usr)
{
}

void Server::executePrivmsgCommand(Message& msg, User& usr)
{
	handleMessages(msg.getArgs(), usr.getSocket(), _server);
}

void Server::executeJoinCommand(Message& msg, User& usr)
{
	joinChannel(msg.getArgs(), usr.getSocket(), _server);
}

void Server::executePartCommand(Message& msg, User& usr)
{
}

void Server::executeInviteCommand(Message& msg, User& usr)
{
}

void Server::executeWhoCommand(Message& msg, User& usr)
{
}

void Server::executeQuitCommand(Message& msg, User& usr)
{
	std::string quitMessage =
	    (msg.getArgs().empty() ? "Quit" : msg.getArgs()[0]);
	// Handle leaving from channels and broadcasting the quit message there as
	// well. some loop to go trough user channels
	//
	// need some proper way to disconnect gracefully and handle the pollfd.
	int fd = usr.getSocket();
	users_.erase(fd);
	close(fd);
}

void Server::executeModeCommand(Message& msg, User& usr)
{
}

void Server::executeKickCommand(Message& msg, User& usr)
{
}

void Server::executeNoticeCommand(Message& msg, User& usr)
{
}

void Server::executeTopicCommand(Message& msg, User& usr)
{
}
