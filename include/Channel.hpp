#pragma once

#include <string>
#include <sstream>
#include <istream>
#include <set>
#include <unistd.h>
#include <iostream>

#include "User.hpp"
#include "Logger.hpp"
#include "Server.hpp"

#define MAX_USERS 100 // to check

class Channel
{
private:
	std::string _name;
	std::set<int> _users;
	std::set<int> _operators;
	bool _isInviteOnly;
	bool _restrictionsOnTopic;
	std::string _topic;
	std::string _password;
	size_t _userLimit;
public:
	Channel(std::string name, int cretorFd);
	~Channel();

	std::string getName() const;
	bool getInviteMode() const;
	std::string getTopic() const;

	void addUser(User &usr, std::string channelPassword);
	void displayMessage(int senderFd, std::string msg);
	bool isUserInChannel(int userFd);
	bool isUserAnOperatorInChannel(int userFd);
	
	void setInviteOnly();
	void unsetInviteOnly();
	void setUserLimit(int limit);
	void unsetUserLimit();
	void setPassword(std::string password);
	void unsetPasword();
	void setRestrictionsOnTopic();
	void unsetRestrictionsOnTopic();

	void addOperator(int userFd);
	void removeOperator(int userFd);
	void removeUser(int userFd);
};

// void joinChannel(std::string msg, int clientFd, Server *_server);
// void handleMessage(std::string msg, int clientFd, Server *_server);