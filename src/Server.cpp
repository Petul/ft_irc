/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 09:51:59 by pleander          #+#    #+#             */
/*   Updated: 2025/02/27 18:16:14 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "Channel.hpp"
#include "Logger.hpp"
#include "Message.hpp"
#include "replies.hpp"
#include "sys/socket.h"

Server::Server(std::string server_pass, int server_port)
	: server_pass_{server_pass}, server_port_{server_port}
{
	_server = this;
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
	this->server_pass_ = o.server_pass_;
	this->server_port_ = o.server_port_;
	return (*this);
}

Server* Server::_server = nullptr;

Server::~Server()
{
	for (auto& ch : _channels)
	{
		ch.second.shutDownChannel();
	}
}

void Server::handleSignal(int signum)
{
	// Dont kill the server for SIGPIPE
	if (signum == 13)
	{
		return;
	}
	static_cast<void>(signum);

	Logger::log(Logger::INFO, "Server shutting down. Goodbye..");
	if (_server) close(_server->_serverSocket);
	_server->~Server();
	exit(0);
}
const std::map<COMMANDTYPE, Server::executeFunc> Server::execute_map_ = {
	{PASS, &Server::pass},
	{NICK, &Server::nick},
	{USER, &Server::user},
	{OPER, &Server::oper},
	{PRIVMSG, &Server::privmsg},
	{JOIN, &Server::join},
	{PART, &Server::part},
	{INVITE, &Server::invite},
	{WHO, &Server::who},
	{WHOIS, &Server::whois},
	{QUIT, &Server::quit},
	{MODE, &Server::mode},
	{KICK, &Server::kick},
	{NOTICE, &Server::notice},
	{TOPIC, &Server::topic},
	{PING, &Server::ping},
	{PONG, &Server::pong},
	{AWAY, &Server::away},
	{NAMES, &Server::names}
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

	signal(SIGPIPE, handleSignal);
	signal(SIGINT, handleSignal);
	signal(SIGQUIT, handleSignal);
}

void Server::startServer()
{
	initServer();
	while (true)
	{
		if (poll(poll_fds_.data(), poll_fds_.size(), -1) < 0)
		{
			throw std::runtime_error(strerror(errno));
		}

		for (size_t i = 0; i < poll_fds_.size(); i++)
		{
			if (poll_fds_[i].revents & POLLIN)
			{
				try
				{
					if (poll_fds_[i].fd == _serverSocket)
					{
						acceptNewClient();
						Logger::log(Logger::DEBUG,
									"Number of connected clients: " +
										std::to_string(poll_fds_.size() - 1));
					}
					else
					{
						receiveDataFromClient(i);
					}
				}
				catch (std::invalid_argument& e)
				{
					Logger::log(Logger::WARNING, e.what());
				}
				catch (std::exception& e)
				{
					Logger::log(Logger::ERROR, e.what());
				}
			}
		}
		clearDisconnectedClients();
	}
}

void Server::acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int clientSocket =
		accept(_serverSocket, (sockaddr*)&server_addr_, &clientLen);
	if (clientSocket > 0)
	{
		Logger::log(Logger::INFO,
					"New client connected: " + std::to_string(clientSocket));
		poll_fds_.push_back({clientSocket, POLLIN, 0});
		this->users_[clientSocket] = User(clientSocket);
	}
	else
	{
		throw std::runtime_error{strerror(errno)};
	}
}

void Server::receiveDataFromClient(int i)
{
	std::string buf;
	User* usr = &(users_[poll_fds_[i].fd]);
	if (!usr->receiveData())
	{
		std::string msg =
			"Client " + std::to_string(poll_fds_[i].fd) + " disconnected";
		Logger::log(Logger::INFO, msg);
		std::string quitMsg = rplQuit(usr->getNick(), usr->getUsername(),
									  usr->getHost(), "Client disconnect");
		handleQuitServer(quitMsg, *usr);
		return;
	}
	while (usr->getNextMessage(buf))
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

void Server::clearDisconnectedClients()
{
	auto pend =
		std::remove_if(poll_fds_.begin(), poll_fds_.end(),
					   [](struct pollfd& pollfd) { return pollfd.fd == -1; });
	if (pend != poll_fds_.end())
	{
		poll_fds_.erase(pend, poll_fds_.end());
		Logger::log(Logger::DEBUG, "Number of connected clients: " +
									   std::to_string(poll_fds_.size() - 1));
	}
	auto it = users_.begin();
	while (it != users_.end())
	{
		if (it->second.getSocket() == -1)
		{
			it = users_.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void Server::executeCommand(Message& msg, User& usr)
{
	if (!usr.isRegistered() && msg.getType() > 4)
	{
		return;
	}
	std::map<COMMANDTYPE, executeFunc>::const_iterator it =
		execute_map_.find(msg.getType());
	if (it != execute_map_.end())
	{
		(this->*(it->second))(msg, usr);
	}
	else
	{
		usr.sendData(
			errUnknownCommand(SERVER_NAME, usr.getNick(), msg.getRawType()));
	}
}

void Server::pass(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓  ERR_NEEDMOREPARAMS			 ✓	ERR_ALREADYREGISTRED*/

	if (msg.getArgs().size() != 1)
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "PASS"));
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

void Server::nick(Message& msg, User& usr)
{
	// We need to handle these possible errors.
	/*Numeric Replies:*/
	/**/
	/*		 ✓	ERR_NONICKNAMEGIVEN				ERR_ERRONEUSNICKNAME*/
	/*		 ✓	ERR_NICKNAMEINUSE				ERR_NICKCOLLISION*/
	/*			ERR_UNAVAILRESOURCE				ERR_RESTRICTED*/

	if (msg.getArgs().size() != 1)
	{
		usr.sendData(errNoNicknameGiven(SERVER_NAME, ""));
		// return;
		throw std::invalid_argument{"No nick name given"};
	}
	std::string newNick = msg.getArgs()[0];
	if (isNickInUse(newNick))
	{
		usr.sendData(errNicknameInUse(SERVER_NAME, newNick));
		// return;
		throw std::invalid_argument{"Nick already in use"};
	}
	if (!usr.getNick().empty())
	{
		usr.sendData(rplNickChange(usr.getNick(), usr.getUsername(),
								   usr.getHost(), newNick));
	}
	usr.setNick(msg.getArgs()[0]);
	if (!usr.isRegistered())
	{
		attemptRegistration(usr);
	}
}

void Server::user(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓  ERR_NEEDMOREPARAMS			 ✓	ERR_ALREADYREGISTRED*/
	if (msg.getArgs().size() != 4)
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "USER"));
		throw std::invalid_argument{"Invalid number of arguments"};
	}
	usr.setUsername(msg.getArgs().front());
	usr.setRealName(msg.getArgs().back());
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
		// TODO: make usr.getMddes(), getChannelModes() and
		// date
		usr.sendData(rplWelcome(SERVER_NAME, usr.getNick(), usr.getUsername(),
								usr.getHost()));
		usr.sendData(rplYourHost(SERVER_NAME, usr.getNick(), SERVER_VER));
		usr.sendData(rplCreated(SERVER_NAME, usr.getNick(),
								"today"));  // maybe we need date.
		usr.sendData(rplMyInfo(SERVER_NAME, usr.getNick(), SERVER_VER,
							   User::avail_user_modes,
							   Channel::avail_channel_modes));
	}
	else
	{
		throw std::invalid_argument{"Invalid password"};
	}
}

// Maybe this bloat will get moved to individual .cpp files
void Server::oper(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*✓	ERR_NEEDMOREPARAMS			  ✓	RPL_YOUREOPER*/
	/*	ERR_NOOPERHOST				  ✓ ERR_PASSWDMISMATCH*/
	if (msg.getArgs().size() < 2)
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "OPER"));
		return;
	}
	std::string operName = msg.getArgs()[0];
	std::string operPass = msg.getArgs()[1];

	if (operName != "root" || operPass != "password")
	{
		usr.sendData(errPasswdMismatch(SERVER_NAME, usr.getNick()));
		return;
	}

	usr.setIsIrcOperator();
	usr.sendData(rplYoureOper(SERVER_NAME, usr.getNick()));
}

void Server::privmsg(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*✓	ERR_NORECIPIENT				✓ ERR_NOTEXTTOSEND*/
	/*✓	ERR_CANNOTSENDTOCHAN		  ERR_NOTOPLEVEL*/
	/*	ERR_WILDTOPLEVEL			✓ ERR_TOOMANYTARGETS*/
	/*✓	ERR_NOSUCHNICK*/
	/*✓	RPL_AWAY*/

	std::vector<std::string> args = msg.getArgs();
	if (args[0].empty())
	{
		usr.sendData(errNoRecipient(SERVER_NAME, usr.getNick(), "PRIVMSG"));
		return;
	}
	if (args.size() < 2 || args[1].empty())
	{
		usr.sendData(errNoTextToSend(SERVER_NAME, usr.getNick()));
		return;
	}

	std::string targets = args[0];
	std::string message = args[1];
	std::istringstream targetStream(targets);
	std::string target;
	std::vector<std::string> allTargets;
	std::set<std::string> usedTargets;
	int targetCount = 0;

	while (std::getline(targetStream, target, ','))
	{
		if (target.empty())
		{
			continue;
		}
		targetCount++;
		if (targetCount > TARGETS_LIM_IN_ONE_COMMAND)
		{
			usr.sendData(errTooManyTargets(SERVER_NAME, usr.getNick(),
										   "Too many targets."));
			return;
		}
		if (!usedTargets.insert(target).second)
		{
			usr.sendData(errTooManyTargets(
				SERVER_NAME, usr.getNick(),
				"Duplicate recipients. No message delivered"));
			return;
		}
		allTargets.push_back(target);
	}

	for (auto target : allTargets)
	{
		if (target.empty())
		{
			continue;
		}

		// If the target starts with ('#'), treat as channel.
		if (target[0] == '#')
		{
			auto chanIt = _channels.find(target);
			if (chanIt == _channels.end())
			{
				usr.sendData(
					errNoSuchChannel(SERVER_NAME, usr.getNick(), target));
			}
			else
			{
				if (!chanIt->second.isUserInChannel(usr))
				{
					usr.sendData(errCannotSendToChan(SERVER_NAME, usr.getNick(),
													 target));
				}
				else
				{
					chanIt->second.displayChannelMessage(usr, message);
				}
			}
		}
		else
		{
			// Otherwise, treat as a private message to a user.
			bool found = false;
			for (auto& userPair : users_)
			{
				if (userPair.second.getNick() == target)
				{
					std::string fullMsg =
						rplPrivMsg(usr.getNick(), target, message);
					if (!userPair.second.getAwayMsg().empty())
					{
						usr.sendData(rplAway(SERVER_NAME, usr.getNick(), target,
											 userPair.second.getAwayMsg()));
					}
					userPair.second.sendData(fullMsg);
					found = true;
					break;
				}
			}
			if (!found)
			{
				usr.sendData(errNoSuchNick(SERVER_NAME, usr.getNick(), target));
			}
		}
	}
}

void Server::join(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*		 ✓	ERR_NEEDMOREPARAMS			   ERR_BANNEDFROMCHAN*/
	/*		 ✓	ERR_INVITEONLYCHAN			✓  ERR_BADCHANNELKEY*/
	/*		 ✓	ERR_CHANNELISFULL			   ERR_BADCHANMASK*/
	/*		 ✓	ERR_NOSUCHCHANNEL			   ERR_TOOMANYCHANNELS*/
	/*		 ✓  ERR_TOOMANYTARGETS			   ERR_UNAVAILRESOURCE*/
	/*		 ✓	RPL_TOPIC*/
	if (msg.getArgs().empty())
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "JOIN"));
		return;
	}

	std::string channelArg = msg.getArgs()[0];
	std::string passwordArg =
		(msg.getArgs().size() > 1) ? msg.getArgs()[1] : "";

	std::istringstream channelStream(channelArg);
	std::istringstream passwordStream(passwordArg);
	std::string channelName, channelPassword;

	int channelCount = 0;
	while (std::getline(channelStream, channelName, ','))
	{
		channelCount++;
		if (channelCount > TARGETS_LIM_IN_ONE_COMMAND)
		{
			usr.sendData(errTooManyTargets(SERVER_NAME, usr.getNick(),
										   "Too many targets."));
			return;
		}
	}

	channelStream.clear();
	channelStream.str(channelArg);
	while (std::getline(channelStream, channelName, ','))
	{
		if (!passwordStream.eof())
		{
			if (!std::getline(passwordStream, channelPassword, ','))
			{
				channelPassword.clear();
			}
		}
		else
		{
			channelPassword.clear();
		}

		if (channelName.empty() || channelName[0] != '#')
		{
			usr.sendData(
				errNoSuchChannel(SERVER_NAME, usr.getNick(), channelName));
			continue;
		}

		if (usr.getUsrChannelCount() >= USR_CHANNEL_LIMIT)
		{
			usr.sendData(errTooManyChannels(SERVER_NAME, usr.getNick()));
			throw(std::invalid_argument("User channel limit reached"));
		}

		std::unordered_map<std::string, Channel>::iterator it =
			_channels.find(channelName);
		if (it == _channels.end())
		{
			if (_channels.size() >= SERVER_CHANNEL_LIMIT)
			{
				usr.sendData(errTooManyTargets(SERVER_NAME, usr.getNick(),
											   "Server channel limit reached"));
				throw(std::invalid_argument("Server channel limit reached"));
			}
			Channel newChannel(channelName, usr);
			if (!channelPassword.empty())
			{
				newChannel.setPassword(channelPassword);
			}

			_channels.insert(std::make_pair(channelName, newChannel));
			it = _channels.find(channelName);

			Logger::log(Logger::INFO,
						"User " + std::to_string(usr.getSocket()) +
							" created new channel " + channelName);
		}

		Channel& channel = it->second;
		channel.joinUser(SERVER_NAME, usr, channelPassword);
	}
}

void Server::part(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*✓	ERR_NEEDMOREPARAMS			✓ ERR_NOSUCHCHANNEL*/
	/*✓	ERR_NOTONCHANNEL*/
	std::vector<std::string> args = msg.getArgs();
	if (args.empty())
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "PART"));
		return;
	}

	std::string channelsList = args[0];
	std::string partMessage = (args.size() > 1 ? args[1] : usr.getNick());

	std::istringstream iss(channelsList);
	std::string channelName;
	while (std::getline(iss, channelName, ','))
	{
		if (channelName.empty()) continue;

		auto it = _channels.find(channelName);
		if (it == _channels.end())
		{
			usr.sendData(
				errNoSuchChannel(SERVER_NAME, usr.getNick(), channelName));
			continue;
		}
		if (!it->second.isUserInChannel(usr))
		{
			usr.sendData(
				errNotOnChannel(SERVER_NAME, usr.getNick(), channelName));
			continue;
		}
		it->second.partUser(usr, partMessage);
		if (it->second.getUserCount() == 0)
		{
			_channels.erase(it);
		}
	}
}

void Server::quit(Message& msg, User& usr)
{
	std::string quitMsg =
		rplQuit(usr.getNick(), usr.getUsername(), usr.getHost(),
				msg.getArgs().empty() ? "Quit" : msg.getArgs()[0]);
	handleQuitServer(quitMsg, usr);
}

void Server::handleQuitServer(std::string& quitMsg, User& usr)
{
	auto it = _channels.begin();
	while (it != _channels.end())
	{
		if (it->second.isUserInChannel(usr))
		{
			it->second.broadcastToChannel(usr, quitMsg);
			it->second.removeUser(usr);
			if (it->second.getUserCount() == 0)
			{
				it = _channels.erase(it);
			}
		}
		else
		{
			it++;
		}
	}

	int fd = usr.getSocket();
	// Works, but can we make it better?
	for (auto it = poll_fds_.begin(); it != poll_fds_.end(); it++)
	{
		if (it->fd == fd)
		{
			it->fd =
				-1;  // Will be deleted by main loop. Don't erase here directly.
			break;
		}
	}
	usr.sendData(quitMsg);
	users_.at(fd).markUserForDeletion();
	close(fd);
}

void Server::mode(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*✓ ERR_NEEDMOREPARAMS			  ✓ ERR_KEYSET*/
	/*	ERR_NOCHANMODES | Not needed? ✓ ERR_CHANOPRIVSNEEDED*/
	/*✓ ERR_USERNOTINCHANNEL		  ✓ ERR_UNKNOWNMODE*/
	/*✓ RPL_CHANNELMODEIS*/
	if (msg.getArgs().empty())
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "MODE"));
		return;
	}

	std::string target = msg.getArgs()[0];

	if (!target.empty() && target[0] == '#')
	{
		handleChannelMode(msg, usr);
	}
	else
	{
		handleUserMode(msg, usr);
	}
}

void Server::handleChannelMode(Message& msg, User& usr)
{
	if (msg.getArgs().size() < 1)
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "MODE"));
		return;
	}

	else if (msg.getArgs().size() == 1)
	{
		std::string channel_name{msg.getArgs()[0]};
		auto it = _channels.find(channel_name);
		if (it == _channels.end())
		{
			usr.sendData(
				errNoSuchChannel(SERVER_NAME, usr.getNick(), channel_name));
			return;
		}
		Channel& ch = it->second;
		usr.sendData(rplChannelModeIs(SERVER_NAME, usr.getNick(), ch.getName(),
									  ch.getChannelModes()));
	}
	else
	{
		std::string channel_name{msg.getArgs()[0]};
		std::string modes = {msg.getArgs()[1]};
		// If there’s a 3rd argument (like a limit or password), store it
		std::string param = (msg.getArgs().size() >= 3) ? msg.getArgs()[2] : "";
		auto it = _channels.find(channel_name);
		if (it == _channels.end())
		{
			usr.sendData(
				errNoSuchChannel(SERVER_NAME, usr.getNick(), channel_name));
			return;
		}
		Channel& ch = it->second;
		if (!ch.isUserInChannel(usr))
		{
			usr.sendData(
				errNotOnChannel(SERVER_NAME, usr.getNick(), channel_name));
			return;
		}
		if (!ch.isUserAnOperatorInChannel(usr))
		{
			usr.sendData(
				errChanPrivsNeeded(SERVER_NAME, usr.getNick(), channel_name));
			return;
		}
		ch.applyChannelMode(usr, modes, param);
	}
}

void Server::handleUserMode(Message& msg, User& usr)
{
	std::vector<std::string> args = msg.getArgs();

	std::string targetNick = args[0];
	std::string modes = (args.size() >= 2) ? args[1] : "";
	std::string param = (args.size() >= 3) ? args[2] : "";

	User* targetUser = NULL;
	for (std::unordered_map<int, User>::iterator it = users_.begin();
		 it != users_.end(); ++it)
	{
		if (it->second.getNick() == targetNick)
		{
			targetUser = &(it->second);
			break;
		}
	}
	if (!targetUser)
	{
		usr.sendData(errNoSuchNick(SERVER_NAME, usr.getNick(), targetNick));
		return;
	}
	targetUser->applyUserMode(usr, modes, param);
}

void Server::topic(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓ ERR_NEEDMOREPARAMS			✓ ERR_NOTONCHANNEL*/
	/*	✓ RPL_NOTOPIC					✓ RPL_TOPIC*/
	/*	✓ ERR_CHANOPRIVSNEEDED			ERR_NOCHANMODES | not needed*/
	std::vector<std::string> args = msg.getArgs();
	if (args.empty())
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "TOPIC"));
		return;
	}
	std::string channelName = args[0];
	std::string newTopic = (args.size() > 1) ? args[1] : "";
	try
	{
		_channels.at(args[0]).showOrSetTopic(usr, newTopic, newTopic.empty());
	}
	catch (const std::exception& e)
	{
		usr.sendData(errNoSuchChannel(SERVER_NAME, usr.getNick(), channelName));
	}
}

void Server::invite(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓ ERR_NEEDMOREPARAMS				✓ ERR_NOSUCHNICK*/
	/*	✓ ERR_NOTONCHANNEL					✓ ERR_USERONCHANNEL*/
	/*	✓ ERR_CHANOPRIVSNEEDED*/
	/*	✓ RPL_INVITING                    	✓ RPL_AWAY*/
	std::vector<std::string> args = msg.getArgs();
	if (args.size() < 2)
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "INVITE"));
		return;
	}
	if (_channels.find(args[1]) == _channels.end())
	{
		for (auto user : users_)
		{
			if (user.second.getNick() == args[0])
			{
				usr.sendData(rplInviting(SERVER_NAME, usr.getNick(), args[0], args[1]));
				user.second.sendData(":" + usr.getNick() + " INVITE " +
						args[0] + " " + args[1] + "\r\n");
				return;
			}
		}
		usr.sendData(errNoSuchNick(SERVER_NAME, usr.getNick(), args[0]));
		return;
	}
	try
	{
		_channels.at(args[1]).inviteUser(usr, users_, args[0]);
	}
	catch (std::exception& e)
	{
		//usr.sendData(errNoSuchChannel(SERVER_NAME, usr.getNick(), args[0]));
	}
}

void Server::kick(Message& msg, User& usr)
{
	/*Numeric Replies:*/
	/**/
	/*	✓ ERR_NEEDMOREPARAMS			✓ ERR_NOSUCHCHANNEL*/
	/*	✓ ERR_BADCHANMASK				✓ ERR_CHANOPRIVSNEEDED*/
	/*	✓ ERR_USERNOTINCHANNEL			✓ ERR_NOTONCHANNEL*/
	/**/

	std::vector<std::string> args = msg.getArgs();
	if (args.size() < 2)
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "KICK"));
		return;
	}
	if (args[0][0] != '#')
	{
		usr.sendData(errBadChanMask(SERVER_NAME, usr.getNick(), "KICK"));
		return;
	}
	try
	{
		_channels.at(args[0]).kickUser(usr, args[1],
									   args.size() == 3 ? args[2] : "");
	}
	catch (std::exception& e)
	{
		usr.sendData(errNoSuchChannel(SERVER_NAME, usr.getNick(), args[0]));
	}
}

void Server::notice(Message& msg, User& usr)
{
	// not sure do we implent this
}

void Server::who(Message& msg, User& usr)
{
	std::vector<std::string> args = msg.getArgs();
	std::string mask;

	if (!args.empty()) mask = args[0];

	if (mask.empty() || mask == "*")
	{
		for (auto user : users_)
		{
			usr.sendData(rplWhoReply(
				SERVER_NAME, usr.getNick(), "*", user.second.getUsername(),
				user.second.getHost(), SERVER_NAME, user.second.getNick(), 'H',
				user.second.getRealname()));
		}
		usr.sendData(rplEndOfWho(SERVER_NAME, usr.getNick(), "*"));
		return;
	}

	if (mask[0] == '#')
	{
		std::unordered_map<std::string, Channel>::iterator chanIt =
			_channels.find(mask);
		if (chanIt == _channels.end())
		{
			usr.sendData(rplEndOfWho(SERVER_NAME, usr.getNick(), mask));
			return;
		}
		Channel& channel = chanIt->second;
		for (auto chanUser : channel.getUsers())
		{
			usr.sendData(rplWhoReply(
				SERVER_NAME, usr.getNick(), mask, chanUser->getUsername(),
				chanUser->getHost(), SERVER_NAME, chanUser->getNick(), 'H',
				chanUser->getRealname()));
		}
		usr.sendData(rplEndOfWho(SERVER_NAME, usr.getNick(), mask));
		return;
	}

	for (auto user : users_)
	{
		if (user.second.getNick() == mask)
		{
			usr.sendData(rplWhoReply(
				SERVER_NAME, usr.getNick(), mask, user.second.getUsername(),
				user.second.getHost(), SERVER_NAME, user.second.getNick(), 'H',
				user.second.getRealname()));
			break;
		}
	}
	// I think we need to send this even if user was not found?
	usr.sendData(rplEndOfWho(SERVER_NAME, usr.getNick(), mask));
}

void Server::whois(Message& msg, User& usr)
{
	std::vector<std::string> args = msg.getArgs();
	if (args.empty())
	{
		usr.sendData(errNeedMoreParams(SERVER_NAME, usr.getNick(), "WHOIS"));
		return;
	}

	std::string targetNick = args[0];
	bool found = false;

	for (auto user : users_)
	{
		if (user.second.getNick() == targetNick)
		{
			found = true;
			usr.sendData(
				rplWhoisUser(SERVER_NAME, usr.getNick(), user.second.getNick(),
							 user.second.getUsername(), user.second.getHost(),
							 user.second.getRealname()));

			if (user.second.getIsIrcOperator())
			{
				usr.sendData(rplWhoisOperator(SERVER_NAME, usr.getNick(),
											  user.second.getNick()));
			}
			usr.sendData(rplEndOfWhois(SERVER_NAME, usr.getNick(),
									   user.second.getNick()));
			break;
		}
	}

	if (!found)
	{
		usr.sendData(errNoSuchNick(SERVER_NAME, usr.getNick(), targetNick));
	}
}

void Server::ping(Message& msg, User& usr)
{
	if (msg.getArgs().empty())
	{
		Logger::log(Logger::WARNING,
					"PING command received with no parameters from user " +
						usr.getNick());
		return;
	}
	std::string pongResponse = std::string(":") + SERVER_NAME + " PONG " +
							   SERVER_NAME + " :" + msg.getArgs()[0] + "\r\n";

	usr.sendData(pongResponse);
	Logger::log(Logger::DEBUG, "Sent PONG to user " + usr.getNick());
}

void Server::pong(Message& msg, User& usr)
{
	Logger::log(Logger::DEBUG, "Received PONG from user " + usr.getNick());
}

void Server::away(Message& msg, User& usr)
{
	std::vector<std::string> args = msg.getArgs();
	usr.setAwayMsg(!args.empty() ? args[0] : "");
}

void Server::names(Message& msg, User& usr)
{
    std::vector<std::string> args = msg.getArgs();
    
    if (args.empty())
    {
        for (auto& chPair : _channels)
        {
            chPair.second.printNames(usr);
        }
        
        std::string lonelyUsers;
        for (auto& userPair : users_)
        {
            User& u = userPair.second;
            if (u.getUsrChannelCount() == 0 && !u.hasMode('i'))
            {
                lonelyUsers += u.getNick() + " ";
            }
        }
        if (!lonelyUsers.empty())
        {
            usr.sendData(rplNamReply(SERVER_NAME, usr.getNick(), "=", "*", lonelyUsers));
            usr.sendData(rplEndOfNames(SERVER_NAME, usr.getNick(), "*"));
        }
        return;
    }
    
    std::string channelList = args[0];
    std::istringstream channelStream(channelList);
    std::string channelName;
    
    while (std::getline(channelStream, channelName, ','))
    {
        auto it = _channels.find(channelName);
        if (it != _channels.end())
        {
            it->second.printNames(usr);
        }
        else
        {
            usr.sendData(rplNamReply(SERVER_NAME, usr.getNick(), "=", channelName, ""));
            usr.sendData(rplEndOfNames(SERVER_NAME, usr.getNick(), channelName));
        }
    }
}
