/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:26:34 by pleander          #+#    #+#             */
/*   Updated: 2025/02/18 02:28:34 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// class Channel;  // Temp

#define PASS_MIN_LEN 4

#include <map>
#include <string>

#include "Message.hpp"
#include "User.hpp"

class Server
{
   public:
	Server();
	Server(std::string server_pass, int server_port, std::string server_name);
	Server(const Server&);
	Server& operator=(const Server&);

	void startServer();
	void sendReply(User& usr, int numeric, const std::string& command, const std::string& message);

   private:
	typedef void (Server::*executeFunc)(Message&, User&);
	static const std::map<COMMANDTYPE, executeFunc> execute_map_;
	void executeCommand(Message& msg, User& usr);
	void executePassCommand(Message& msg, User& usr);
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
	std::string server_name_;
	std::string server_pass_;
	int server_port_;
	// std::map<std::string, Channel> channels_;
	std::map<int, User> users_;
};
