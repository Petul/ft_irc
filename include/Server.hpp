/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:26:34 by pleander          #+#    #+#             */
/*   Updated: 2025/02/27 15:03:02 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#define PASS_MIN_LEN 4
#define SERVER_VER "0.1"
#define SERVER_NAME "ircserv"
#define TARGETS_LIM_IN_ONE_COMMAND 15
#define SERVER_CHANNEL_LIMIT 50
#define USR_CHANNEL_LIMIT 25

#include <netinet/in.h>
#include <poll.h>

#include <string>
#include <unordered_map>

#include "Channel.hpp"
#include "Message.hpp"
#include "User.hpp"
#include "replies.hpp"

class Server
{
   public:
	Server(std::string server_pass, int server_port);
	Server(const Server&);
	Server& operator=(const Server&);
	~Server();

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

	std::unordered_map<int, User> users_;
	static Server* _server;
	std::unordered_map<std::string, Channel> _channels;

	int _serverSocket;
	struct sockaddr_in server_addr_;
	std::vector<struct pollfd> poll_fds_;

	void parseMessage(std::string& msg, int clientFd, Server* _server);
	COMMANDTYPE getMessageType(std::string& msg);

	void executeCommand(Message& msg, User& usr);
	void pass(Message& msg, User& usr);
	bool isNickInUse(std::string& nick);
	void nick(Message& msg, User& usr);
	void user(Message& msg, User& usr);
	void attemptRegistration(User& usr);
	void oper(Message& msg, User& usr);
	void privmsg(Message& msg, User& usr);
	void join(Message& msg, User& usr);
	void part(Message& msg, User& usr);
	void invite(Message& msg, User& usr);
	void who(Message& msg, User& usr);
	void whois(Message& msg, User& usr);
	void quit(Message& msg, User& usr);
	void mode(Message& msg, User& usr);
	void kick(Message& msg, User& usr);
	void notice(Message& msg, User& usr);
	void topic(Message& msg, User& usr);
	void ping(Message& msg, User& usr);
	void pong(Message& msg, User& usr);
	void away(Message& msg, User& usr);
	void names(Message& msg, User& usr);

	void handleChannelMode(Message& msg, User& usr);
	void handleUserMode(Message& msg, User& usr);
	void handleQuitServer(std::string& quitMsg, User& usr);
};
