/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 09:51:59 by pleander          #+#    #+#             */
/*   Updated: 2025/02/20 09:52:07 by pleander         ###   ########.fr       */
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
#include "replies.hpp"
#include "sys/socket.h"

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
	{PING, &Server::executePingCommand},
	{PONG, &Server::executePongCommand}
	// Extend this list when we have more functions
};

void Server::initServer()
{
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error("failed to create socket");

	// Enable port reuse
	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
				   sizeof(opt)) < 0)
	{
		throw std::runtime_error("setsockopt");
	}

	server_addr_.sin_family = AF_INET;
	server_addr_.sin_addr.s_addr = INADDR_ANY;
	server_addr_.sin_port = htons(this->server_port_);

	if (bind(_serverSocket, (sockaddr*)&server_addr_, sizeof(server_addr_)) ==
		-1)
		throw std::runtime_error("failed to bind socket");

	if (listen(_serverSocket, 5) == -1)
		throw std::runtime_error("listen() failed");
	Logger::log(Logger::INFO, "Server listening on port " +
								  std::to_string(this->server_port_));
	poll_fds_.push_back({_serverSocket, POLLIN, 0});

	signal(SIGINT, handleSignal);  //
}

void Server::startServer()
{
	initServer();
	while (1)
	{
		int eventCount = poll(poll_fds_.data(), poll_fds_.size(), -1);
		if (eventCount == -1)
		{
			throw std::runtime_error("Error: poll");
		}

		for (size_t i = 0; i < poll_fds_.size(); i++)
		{
			if (poll_fds_[i].revents & POLLIN)
			{
				if (poll_fds_[i].fd == _serverSocket)
				{  // new client trying to connect
					struct sockaddr_in clientAddr;
					socklen_t clientLen = sizeof(clientAddr);
					int clientSocket = accept(
						_serverSocket, (sockaddr*)&server_addr_, &clientLen);
					if (clientSocket > 0)
					{
						Logger::log(Logger::INFO,
									"New client connected: " +
										std::to_string(clientSocket));
						poll_fds_.push_back({clientSocket, POLLIN, 0});
						this->users_[clientSocket] = User(clientSocket);
					}
				}
				else
				{
					try
					{
						std::string buf;
						User* user = &(users_[poll_fds_[i].fd]);
						if (!user->receiveData())
						{
							std::string msg = "Client " +
											  std::to_string(poll_fds_[i].fd) +
											  " disconnected";
							Logger::log(Logger::INFO, msg);

							poll_fds_[i].fd =
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
								executeCommand(msg, users_[poll_fds_[i].fd]);
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
}

std::map<std::string, Channel>& Server::getChannels()
{
	return _channels;
}
std::map<int, User>& Server::getUsers()
{
	return users_;
};

void Server::executeCommand(Message& msg, User& usr)
{
	// if (!usr.isRegistered() && msg.getType() > 4) // commented out just for
	// testing
	// {
	//	return;
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

void Server::executePassCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓  ERR_NEEDMOREPARAMS			 ✓	ERR_ALREADYREGISTRED*/

	if (msg.getArgs().size() != 1)
	{
		usr.sendData(errNeedMoreParams(server_name_, usr.getNick(), "PASS"));
		throw std::invalid_argument{"Invalid number of arguments"};
	}
	else if (usr.isRegistered())
	{
		usr.sendData(errAlreadyRegistered());
	}
	usr.setPassword(msg.getArgs()[0]);
	Logger::log(Logger::DEBUG, "Set user password to " + msg.getArgs()[0]);
}

bool Server::isNickInUse(std::string& nick)
{
	for (auto& entry : users_)
	{
		if (entry.second.getNick() == nick)
		{
			return true;
		}
	}
	return false;
}

void Server::executeNickCommand(Message& msg, User& usr)
{
	// We need to handle these possible errors.
	/*Numeric Replies:*/
	/**/
	/*		 ✓	ERR_NONICKNAMEGIVEN				ERR_ERRONEUSNICKNAME*/
	/*		 ✓	ERR_NICKNAMEINUSE				ERR_NICKCOLLISION*/
	/*			ERR_UNAVAILRESOURCE				ERR_RESTRICTED*/

	if (msg.getArgs().size() != 1)
	{
		usr.sendData(errNoNicknameGiven(server_name_, ""));
		// return;
		throw std::invalid_argument{"No nick name given"};
	}
	std::string newNick = msg.getArgs()[0];
	if (isNickInUse(newNick))
	{
		usr.sendData(errNicknameInUse(server_name_, newNick));
		// return;
		throw std::invalid_argument{"Nick already in use"};
	}
	if (!usr.getNick().empty())
	{
		usr.sendData(rplNickChange(usr.getNick(), usr.getUsername(),
								   "TODO:MAKEGETHOST", newNick));
	}
	usr.setNick(msg.getArgs()[0]);
	if (!usr.isRegistered())
	{
		attemptRegistration(usr);
	}
}

void Server::executeUserCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓  ERR_NEEDMOREPARAMS			 ✓	ERR_ALREADYREGISTRED*/
	if (msg.getArgs().size() != 4)
	{
		usr.sendData(errNeedMoreParams(server_name_, usr.getNick(), "USER"));
		throw std::invalid_argument{"Invalid number of arguments"};
	}
	usr.setUsername(msg.getArgs().front());
	if (!usr.isRegistered())
	{
		attemptRegistration(usr);
	}
	else
	{
		usr.sendData(errAlreadyRegistered());
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
		// TODO: make usr.getMddes(), getChannelModes() usr.getHost() and date
		usr.sendData(rplWelcome(server_name_, usr.getNick(), usr.getUsername(),
								"usr.getHost()"));
		usr.sendData(rplYourHost(server_name_, usr.getNick(), SERVER_VER));
		usr.sendData(rplCreated(server_name_, usr.getNick(),
								"today"));  // maybe we need date.
		usr.sendData(rplMyInfo(server_name_, usr.getNick(), SERVER_VER,
							   "usr.getModes()", "getChannelModes()"));
	}
	else
	{
		throw std::invalid_argument{"Invalid password"};
	}
}

// Maybe this bloat will get moved to individual .cpp files
void Server::executeOperCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NEEDMOREPARAMS				RPL_YOUREOPER*/
	/*	ERR_NOOPERHOST					ERR_PASSWDMISMATCH*/
}

void Server::executePrivmsgCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NORECIPIENT					ERR_NOTEXTTOSEND*/
	/*	ERR_CANNOTSENDTOCHAN			ERR_NOTOPLEVEL*/
	/*	ERR_WILDTOPLEVEL				ERR_TOOMANYTARGETS*/
	/*	ERR_NOSUCHNICK*/
	/*	RPL_AWAY*/

	// handleMessages(msg.getArgs(), usr.getSocket(), _server);
	std::vector<std::string> args = msg.getArgs();
	std::string channel, message;  // possible to have multiple messages?
	int clientFd = usr.getSocket();

	channel = args[0];
	for (size_t i = 1; i < args.size(); i++)
	{
		if (i > 1) message += " ";
		message += args[i];
	}
	if (!channel.empty() && !message.empty())
	{
		auto it = _channels.find(channel);
		if (it != _channels.end() && it->second.isUserInChannel(usr))
		{
			it->second.displayMessage(usr, message);
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

void Server::executeJoinCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*			ERR_NEEDMOREPARAMS				ERR_BANNEDFROMCHAN*/
	/*			ERR_INVITEONLYCHAN				ERR_BADCHANNELKEY*/
	/*			ERR_CHANNELISFULL				ERR_BADCHANMASK*/
	/*			ERR_NOSUCHCHANNEL				ERR_TOOMANYCHANNELS*/
	/*			ERR_TOOMANYTARGETS				ERR_UNAVAILRESOURCE*/
	/*			RPL_TOPIC*/
	// joinChannel(msg.getArgs(), usr.getSocket(), _server);
	std::vector<std::string> args = msg.getArgs();
	int clientFd = usr.getSocket();
	std::vector<std::pair<std::string, std::string>> channelsAndPasswords;
	std::stringstream ssChannel(args[0]);
	std::stringstream ssPassword(args.size() > 1 ? args[1] : "");
	std::string channel, password;
	while (std::getline(ssChannel, channel, ','))
	{
		if (std::getline(ssPassword, password, ','))
			channelsAndPasswords.push_back({channel, password});
		else
			channelsAndPasswords.push_back({channel, ""});
	}
	for (size_t i = 0; i < channelsAndPasswords.size(); i++)
	{
		std::string channelName = channelsAndPasswords[i].first;
		std::string channelPassword = channelsAndPasswords[i].second;
		if (channelName.empty() || channelName[0] != '#')
		{
			std::cout << "Invalid channel name: " << channelName << std::endl;
			continue;
		}
		auto it = _channels.find(channelName);
		if (it == _channels.end())
		{
			Channel newChannel(channelName, usr);
			_channels.insert({newChannel.getName(), newChannel});
			Logger::log(Logger::INFO, "user " + std::to_string(clientFd) +
										  " created new channel " +
										  channelName);
			it = _channels.find(channelName);
		}
		else
		{
			if (it->second.getInviteMode() == false)
				it->second.addUser(usr, channelPassword);
			else
			{
				if (it->second.checkIfUserInvited(usr))
					it->second.addUser(usr, channelPassword);
				else
					Logger::log(Logger::WARNING,
								"channel " + channelName +
									" is in invite-only mode, user " +
									std::to_string(clientFd) + "can't join");
			}
		}
	}
}

void Server::executePartCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NEEDMOREPARAMS				ERR_NOSUCHCHANNEL*/
	/*	ERR_NOTONCHANNEL*/
}

void Server::executeQuitCommand(Message& msg, User& usr)
{
	std::string quitMessage =
		(msg.getArgs().empty() ? "Quit" : msg.getArgs()[0]);
	// TODO: Handle leaving from channels and broadcasting the quit message
	// there as
	//	well. some loop to go trough user channels
	//
	//	need some proper way to disconnect gracefully and handle the pollfd.
	usr.sendData(rplQuit(usr.getNick(), quitMessage));
	int fd = usr.getSocket();
	users_.erase(fd);
	close(fd);
}

// maybe all the logs could be moved inside each function
void Server::executeModeCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NEEDMOREPARAMS				ERR_KEYSET*/
	/*	ERR_NOCHANMODES					ERR_CHANOPRIVSNEEDED*/
	/*	ERR_USERNOTINCHANNEL			ERR_UNKNOWNMODE*/
	/*	RPL_CHANNELMODEIS*/
	/*	RPL_BANLIST						RPL_ENDOFBANLIST*/
	/*	RPL_EXCEPTLIST					RPL_ENDOFEXCEPTLIST*/
	/*	RPL_INVITELIST					RPL_ENDOFINVITELIST*/
	/*	RPL_UNIQOPIS*/
	std::vector<std::string> args = msg.getArgs();
	// for (size_t i= 0; i < args.size(); i++)
	//	std::cout << args[i] << std::endl;
	std::string channel, mode, parameter;
	channel = args[0];
	mode = args[1];  // also thi is not gonna work since we can have something
					 // like this as input: MODE #example +i +t +l 5
	parameter =
		(args.size() == 3 ? args[2] : "");  // i don't like this...to change
	auto it = _channels.find(channel);
	if (it != _channels.end() && it->second.isUserAnOperatorInChannel(usr))
	{
		if (mode.find('i') != std::string::npos)
		{
			if (mode == "+i")
			{
				it->second.setInviteOnly();
				Logger::log(Logger::DEBUG,
							"user " + usr.getUsername() +
								" set invite only mode ON in channel " +
								channel);
			}
			if (mode == "-i")
			{
				it->second.unsetInviteOnly();
				Logger::log(Logger::DEBUG,
							"user " + usr.getUsername() +
								" set invite only mode OFF in channel " +
								channel);
			}
		}
		else if (mode.find('t') != std::string::npos)
		{
			if (mode == "+t")
			{
				it->second.setRestrictionsOnTopic();
				Logger::log(
					Logger::DEBUG,
					"user " + usr.getUsername() +
						" set restrictions on topic mode ON in channel " +
						channel);
			}
			if (mode == "-t")
			{
				it->second.unsetRestrictionsOnTopic();
				Logger::log(
					Logger::DEBUG,
					"user " + usr.getUsername() +
						" set restrictions on topic mode OFF in channel " +
						channel);
			}
		}
		else if (mode.find('k') != std::string::npos)
		{
			if (mode == "+k")
			{
				it->second.setPassword(parameter);
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() +
											   " set password in channel " +
											   channel);
			}
			if (mode == "-k")
			{
				it->second.unsetPasword();
				Logger::log(Logger::DEBUG, "user " + usr.getUsername() +
											   " removed password in channel " +
											   channel);
			}
		}
		else if (mode.find('o') != std::string::npos)
		{
			for (auto itU = users_.begin(); itU != users_.end(); itU++)
			{
				if (itU->second.getUsername() == parameter)
				{
					if (mode == "+o" && it->second.isUserInChannel(itU->second))
					{
						it->second.addOperator(itU->second);
						Logger::log(
							Logger::DEBUG,
							"user " + usr.getUsername() +
								" gave operator privilege for channel " +
								channel + " to user " + usr.getUsername());
					}
					if (mode == "-o" &&
						it->second.isUserAnOperatorInChannel(itU->second))
					{
						it->second.removeOperator(itU->second);
						Logger::log(
							Logger::DEBUG,
							"user " + usr.getUsername() +
								" revoked operator privilege for channel " +
								channel + " to user " + usr.getUsername());
					}
				}
			}
			Logger::log(Logger::DEBUG, "username " + parameter +
										   " not existing or not in channel " +
										   channel);
		}
		else if (mode.find('l') != std::string::npos)
		{
			if (mode == "+l" && std::stoi(parameter))
			{
				it->second.setUserLimit(
					std::stoi(parameter));  // to check/protect
				Logger::log(Logger::DEBUG, "User " + usr.getUsername() +
											   "set channel limit to " +
											   parameter + " users");
			}
			if (mode == "-l")
			{
				it->second.unsetUserLimit();
				Logger::log(Logger::DEBUG, "User " + usr.getUsername() +
											   "unset channel limit");
			}
		}
	}
}

void Server::executeTopicCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NEEDMOREPARAMS				ERR_NOTONCHANNEL*/
	/*	RPL_NOTOPIC						RPL_TOPIC*/
	/*	ERR_CHANOPRIVSNEEDED			ERR_NOCHANMODES*/
}

void Server::executeInviteCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NEEDMOREPARAMS				ERR_NOSUCHNICK*/
	/*	ERR_NOTONCHANNEL				ERR_USERONCHANNEL*/
	/*	ERR_CHANOPRIVSNEEDED*/
	/*	RPL_INVITING                    RPL_AWAY*/
	std::vector<std::string> args = msg.getArgs();
	std::string user, channel;
	user = args[0];
	channel = args[1];
	auto it = _channels.find(channel);
	if (it != _channels.end() && it->second.isUserAnOperatorInChannel(usr))
	{
		auto itU = users_.find(std::stoi(user));
		if (itU != users_.end()) it->second.inviteUser(usr, itU->second);
	}
}

void Server::executeKickCommand(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	ERR_NEEDMOREPARAMS				ERR_NOSUCHCHANNEL*/
	/*	ERR_BADCHANMASK					ERR_CHANOPRIVSNEEDED*/
	/*	ERR_USERNOTINCHANNEL			ERR_NOTONCHANNEL*/
	/**/

	std::vector<std::string> args = msg.getArgs();
	std::string channel, user, reason;
	channel = args[0];
	user = args[1];
	reason = (args.size() == 3 ? args[2] : "");
	auto it = _channels.find(channel);
	if (it != _channels.end() && it->second.isUserAnOperatorInChannel(usr))
	{
		for (auto itU = users_.begin(); itU != users_.end(); itU++)
		{
			if (itU->second.getUsername() ==
				user)  // not sure if username or nickname
			{
				it->second.removeUser(usr, itU->second, reason);
				Logger::log(Logger::DEBUG, "User " + usr.getUsername() +
											   " kicked out user " +
											   itU->second.getUsername() +
											   " from channel " + channel);
			}
		}
	}
}

void Server::executeNoticeCommand(Message& msg, User& usr)
{
	// not sure do we implent this
}

void Server::executeWhoCommand(Message& msg, User& usr)
{
	// not sure do we implent this
}

void Server::executePingCommand(Message& msg, User& usr)
{
	if (msg.getArgs().empty())
	{
		Logger::log(Logger::WARNING,
					"PING command received with no parameters from user " +
						usr.getNick());
		return;
	}
	std::string pongResponse = ":" + server_name_ + " PONG " + server_name_ +
							   " :" + msg.getArgs()[0] + "\r\n";

	usr.sendData(pongResponse);
	Logger::log(Logger::DEBUG, "Sent PONG to user " + usr.getNick());
}

void Server::executePongCommand(Message& msg, User& usr)
{
	Logger::log(Logger::DEBUG, "Received PONG from user " + usr.getNick());
}
