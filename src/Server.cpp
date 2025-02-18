/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 08:39:33 by pleander          #+#    #+#             */
/*   Updated: 2025/02/18 14:13:15 by mpellegr         ###   ########.fr       */
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
#include <csignal>
#include <iostream>

#include "Logger.hpp"
#include "netinet/in.h"
#include "sys/socket.h"
#include "Channel.hpp"
#include "Message.hpp"

Server::Server() : Server("default", 8123, "defaultServerName")
{
}

Server::Server(std::string server_pass, int server_port, std::string server_name)
    : server_pass_{server_pass}, server_port_{server_port}, server_name_{server_name}
{
	_server = this;
}

Server::Server(const Server& o) : Server(o.server_pass_, o.server_port_, o.server_name_)
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

Server *Server::_server = nullptr;

void Server::handleSignal(int signum) {
	static_cast<void>(signum);
	std::cout << "\nServer shutting down...\n";
	if (_server)
		close(_server->_serverSocket);
	exit(0);
}
const std::map<COMMANDTYPE, Server::executeFunc> Server::execute_map_ = {
	{ PASS, &Server::executePassCommand },
	{ NICK, &Server::executeNickCommand },
	{ USER, &Server::executeUserCommand },
	{ OPER, &Server::executeOperCommand },
	{ PRIVMSG, &Server::executePrivmsgCommand },
	{ JOIN, &Server::executeJoinCommand },
	{ PART, &Server::executePartCommand },
	{ INVITE, &Server::executeInviteCommand },
	{ WHO, &Server::executeWhoCommand },
	{ QUIT, &Server::executeQuitCommand },
	{ MODE, &Server::executeModeCommand },
	{ KICK, &Server::executeKickCommand },
	{ NOTICE, &Server::executeNoticeCommand },
	{ TOPIC, &Server::executeTopicCommand },
	// Extend this list when we have more functions
};

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
	// int opt{1};
	// if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	//                sizeof(opt)))
	// {
	// 	throw std::runtime_error("setsockopt");
	// }

	// if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
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
	// close(_serverSocket);  // Never reaching, handle somehow
}

std::map<std::string, Channel> &Server::getChannels() { return _channels; }
std::map<int, User> &Server::getUsers() { return users_; };

// void joinChannel(std::vector<std::string> args, int clientFd, Server *_server) {
// 	std::vector<std::pair<std::string, std::string>> channels;
// 	std::stringstream ssChannel(args[0]);
// 	std::string channel;
// 	std::stringstream ssPassword(args.size() > 1 ? args[1] : "");
// 	std::string password;
// 	while (std::getline(ssChannel, channel, ',')) {
// 		if (std::getline(ssPassword, password, ','))
// 			channels.push_back({channel, password});
// 		else
// 			channels.push_back({channel, ""});
// 	}
// 	for (size_t i = 0; i < channels.size(); i++) {
// 		if (channelName.empty() || channelName[0] != '#') {
// 			std::cout << "Invalid channel name: " << channelName << std::endl;
// 			continue;
// 		}
// 		std::map<std::string, Channel> &existingChannels = _server->getChannels();
// 		auto it = existingChannels.find(channelName);
// 		if (it == existingChannels.end()) {
// 			Channel newChannel(channelName, clientFd);
// 			existingChannels.insert({newChannel.getName(), newChannel});
// 			Logger::log(Logger::INFO, "user " + std::to_string(clientFd) + " created new channel " + channelName);
// 			it = existingChannels.find(channelName);
// 		} else {
// 			if (it->second.getInviteMode() == false)
// 				it->second.addUser(clientFd);
// 			else
// 				Logger::log(Logger::WARNING, "channel " + channelName + " is in invite-only mode, user " + std::to_string(clientFd) + "can't join");
// 		}
// 	}
// }

void handleMessages(std::vector<std::string> messages, int clientFd, Server *_server) {
	std::string channel, message; // possible to have multiple messages?

	channel = messages[0];
	message = messages[1];
	if (!channel.empty() && !message.empty()) {
		std::map<std::string, Channel> &existingChannels = _server->getChannels();
		auto it = existingChannels.find(channel);
		if (it != existingChannels.end() && it->second.isUserInChannel(clientFd)) {
			it->second.displayMessage(clientFd, message);
		} else {
			std::string msg = "User " + std::to_string(clientFd) + " is not in channel " + channel + " or channel doesn't exist.\n";
			Logger::log(Logger::ERROR, msg);
		}
	}
}

void Server::executeCommand(Message& msg, User& usr)
{
	// if (!usr.isRegistered() && msg.getType() > 4) // commented out just for testing
	// {
	// 	return;
	// }
    std::map<COMMANDTYPE, executeFunc>::const_iterator it = execute_map_.find(msg.getType());
    if (it != execute_map_.end())
    {
        (this->*(it->second))(msg, usr);
    }
    else
    {
        throw std::invalid_argument("Unsupported command");
    }
}

//this is actually not good. It's easier to hardcode all the different RPL & ERR msgs.
void Server::sendReply(User& usr, int numeric, const std::string& command, const std::string& message)
{
	std::ostringstream oss;
	oss << ":" << server_name_ << " " 
		<< numeric << " " 
		<< usr.getNick() << " " 
		<< command << " :" 
		<< message;
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
	// joinChannel(msg.getArgs(), usr.getSocket(), _server);
	std::vector<std::string> args = msg.getArgs();
	int clientFd = usr.getSocket();
	std::vector<std::pair<std::string, std::string>> channelsAndPasswords;
	std::stringstream ssChannel(args[0]);
	std::stringstream ssPassword(args.size() > 1 ? args[1] : "");
	std::string channel, password;
	while (std::getline(ssChannel, channel, ',')) {
		if (std::getline(ssPassword, password, ','))
			channelsAndPasswords.push_back({channel, password});
		else
			channelsAndPasswords.push_back({channel, ""});
	}
	for (size_t i = 0; i < channelsAndPasswords.size(); i++) {
		std::string channelName = channelsAndPasswords[i].first;
		std::string channelPassword = channelsAndPasswords[i].second;
		if (channelName.empty() || channelName[0] != '#') {
			std::cout << "Invalid channel name: " << channelName << std::endl;
			continue;
		}
		std::map<std::string, Channel> &existingChannels = _server->getChannels();
		auto it = existingChannels.find(channelName);
		if (it == existingChannels.end()) {
			Channel newChannel(channelName, clientFd);
			existingChannels.insert({newChannel.getName(), newChannel});
			Logger::log(Logger::INFO, "user " + std::to_string(clientFd) + " created new channel " + channelName);
			it = existingChannels.find(channelName);
		} else {
			if (it->second.getInviteMode() == false)
				it->second.addUser(usr, channelPassword);
			else
				Logger::log(Logger::WARNING, "channel " + channelName + " is in invite-only mode, user " + std::to_string(clientFd) + "can't join");
		}
	}
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
	std::string quitMessage = (msg.getArgs().empty() ? "Quit" : msg.getArgs()[0]);
	// Handle leaving from channels and broadcasting the quit message there as well.
	// some loop to go trough user channels
	//
	// need some proper way to disconnect gracefully and handle the pollfd.
	int fd = usr.getSocket();
	users_.erase(fd);
	close(fd);
}

// maybe all the logs could be moved inside each function
void Server::executeModeCommand(Message& msg, User& usr)
{
	std::vector<std::string> args = msg.getArgs();
	// for (size_t i= 0; i < args.size(); i++)
	// 	std::cout << args[i] << std::endl;
	std::string channel, mode, parameter;
	channel = args[0];
	mode = args[1]; // also thi is not gonna work since we can have something like this as input: MODE #example +i +t +l 5
	parameter = (args.size() == 3 ? args[2] : ""); // i don't like this...to change
	std::map<std::string, Channel> &existingChannels = _server->getChannels();
	auto it = existingChannels.find(channel);
	if (it != existingChannels.end() && it->second.isUserAnOperatorInChannel(usr.getSocket())) {
		if (mode.find('i') != std::string::npos) {
			if (mode == "+i") {
				it->second.setInviteOnly();
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " set invite only mode ON in channel " + channel);
			}
			if (mode == "-i") {
				it->second.unsetInviteOnly();
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " set invite only mode OFF in channel " + channel);
			}
		} else if (mode.find('t') != std::string::npos) {
			if (mode == "+t") {
				it->second.setRestrictionsOnTopic();
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " set restrictions on topic mode ON in channel " + channel);
			}
			if (mode == "-t") {
				it->second.unsetRestrictionsOnTopic();
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " set restrictions on topic mode OFF in channel " + channel);
			}
		} else if (mode.find('k') != std::string::npos) {
			if (mode == "+k") {
				it->second.setPassword(parameter);
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " set password in channel " + channel);
			}
			if (mode == "-k") {
				it->second.unsetPasword();
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " removed password in channel " + channel);
			}
		} else if (mode.find('o') != std::string::npos) {
			std::map<int, User> &users = _server->getUsers();
			for (auto itU = users.begin(); itU != users.end(); it++) {
				if (itU->second.getUsername() == parameter) {
					if (mode == "+o" && it->second.isUserInChannel(itU->second.getSocket())) {
						it->second.addOperator(itU->second.getSocket());
						Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " gave operator privilege for channel " + channel + " to user " + usr.getUsername());
					}
					if (mode == "-o" && it->second.isUserAnOperatorInChannel(itU->second.getSocket())) {
						it->second.removeOperator(itU->second.getSocket());
						Logger::log(Logger::DEBUG, "user " + usr.getUsername() + " revoked operator privilege for channel " + channel + " to user " + usr.getUsername());
					}
				}
			}
			Logger::log(Logger::DEBUG, "username " + parameter + " not existing or not in channel " + channel);
		} else if (mode.find('l') != std::string::npos) {
			if (mode == "+l" && std::stoi(parameter)) {
				it->second.setUserLimit(std::stoi(parameter)); // to check/protect
				Logger::log(Logger::DEBUG, "User " + usr.getUsername() + "set channel limit to " + parameter + " users");
			}
			if (mode == "-l") {
				it->second.unsetUserLimit();
				Logger::log(Logger::DEBUG, "User " + usr.getUsername() + "unset channel limit");
			}
		}
	}
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
