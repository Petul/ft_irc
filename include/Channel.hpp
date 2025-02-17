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

class Channel
{
private:
	std::string _name;
	std::set<int> _users;
	std::set<int> _operators;
public:
	Channel(std::string name, int cretorFd);
	~Channel();

	std::string getName() const;
	void addUser(int userFd);
	void displayMessage(int senderFd, std::string msg);
	bool isUserInChannel(int userFd);
};

// void joinChannel(std::string msg, int clientFd, Server *_server);
// void handleMessage(std::string msg, int clientFd, Server *_server);