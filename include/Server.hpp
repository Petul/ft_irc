/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:26:34 by pleander          #+#    #+#             */
/*   Updated: 2025/02/19 00:27:16 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// class Channel;  // Temp

#define PASS_MIN_LEN 4
#define SERVER_VER "0.1"

#include <netinet/in.h>
#include <poll.h>

#include <string>
#include <unordered_map>

#include "Channel.hpp"
#include "Message.hpp"
#include "User.hpp"
#include "replies.hpp"

class Channel;

class Server
{
   public:
	Server(std::string server_pass, int server_port, std::string server_name);
	Server(const Server&);
	Server& operator=(const Server&);

	void startServer();
	static void handleSignal(int signum);
	void sendReply(User& usr, int numeric, const std::string& command,
				   const std::string& message);

   private:
	Server();
	void initServer();
	void acceptNewClient();
	void receiveDataFromClient(int i);
	void clearDisconnectedClients();

	typedef void (Server::*executeFunc)(Message&, User&);
	static const std::map<COMMANDTYPE, executeFunc> execute_map_;

	std::string server_pass_;
	int server_port_;
	std::string server_name_;

	std::unordered_map<int, User> users_;
	static Server* _server;
	std::unordered_map<std::string, Channel> _channels;

	int _serverSocket;
	struct sockaddr_in server_addr_;
	std::vector<struct pollfd> poll_fds_;

	void parseMessage(std::string& msg, int clientFd, Server* _server);
	COMMANDTYPE getMessageType(std::string& msg);

	void executeCommand(Message& msg, User& usr);
	void executePassCommand(Message& msg, User& usr);
	bool isNickInUse(std::string& nick);
	void executeNickCommand(Message& msg, User& usr);
	void executeUserCommand(Message& msg, User& usr);
	void attemptRegistration(User& usr);
	void executeOperCommand(Message& msg, User& usr);
	void executePrivmsgCommand(Message& msg, User& usr);
	void executeJoinCommand(Message& msg, User& usr);
	void executePartCommand(Message& msg, User& usr);
	void executeInviteCommand(Message& msg, User& usr);
	void executeWhoCommand(Message& msg, User& usr);
	void executeQuitCommand(Message& msg, User& usr);
	void executeModeCommand(Message& msg, User& usr);
	void executeKickCommand(Message& msg, User& usr);
	void executeNoticeCommand(Message& msg, User& usr);
	void executeTopicCommand(Message& msg, User& usr);
	void executePingCommand(Message& msg, User& usr);
	void executePongCommand(Message& msg, User& usr);
};
