/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:26:34 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 16:13:25 by mpellegr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// class Channel;  // Temp

#include <map>
#include <string>

#include "Message.hpp"
#include "User.hpp"
#include "Channel.hpp"

class Channel;

class Server
{
   public:
	Server();
	Server(std::string server_pass, int server_port);
	Server(const Server&);
	Server& operator=(const Server&);

	void startServer();
	static void handleSignal(int signum);
	std::map<std::string, Channel> &getChannels();
	std::map<int, User> &getUsers();

   private:
	std::string server_pass_;
	int server_port_;
	// std::map<std::string, Channel> channels_;
	std::map<int, User> users_;
	static Server *_server;
	std::map<std::string, Channel> _channels;

	int _serverSocket;
	void parseMessage(std::string& msg, int clientFd, Server *_server);
	COMMANDTYPE getMessageType(std::string& msg);
};
