#pragma once

#include <unistd.h>

#include <iostream>
#include <istream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Logger.hpp"
#include "User.hpp"

#define MAX_USERS 100  // to check

class Channel
{
   private:
	std::string _name;
	std::set<User *> _users;
	std::set<User *> _operators;
	std::set<User *> _invitedUsers;
	bool _isInviteOnly;
	bool _restrictionsOnTopic;
	std::string _topic;
	std::string _password;
	size_t _userLimit;

   public:
	Channel(std::string name, User &usr);
	~Channel();

	std::string getName() const;
	bool getInviteMode() const;
	std::string getTopic() const;
	unsigned int getUserCount() const;

	void addUser(User &usr, std::string channelPassword);
	// void inviteUser(User &invitingUsr, User &invitedUsr);
	void inviteUser(User &invitingUsr, std::unordered_map<int, User> &users_, std::string invitedUsrNickname);
	void displayMessage(User &sender, std::string msg);
	bool isUserInChannel(User &usr);
	bool isUserAnOperatorInChannel(User &usr);
	bool checkIfUserInvited(User &user);

	void setInviteOnly();
	void unsetInviteOnly();
	void setUserLimit(int limit);
	void unsetUserLimit();
	void setPassword(std::string password);
	void unsetPasword();
	void setRestrictionsOnTopic();
	void unsetRestrictionsOnTopic();
	void showOrSetTopic(User &usr, std::string topic, int unsetTopicFlag);

	void addOperator(User &user);
	void removeUser(User& user);
	void removeOperator(User &user);
	void kickUser(User &user, std::string targetUsername, std::string reason);
	void part(User &usr, const std::string &partMessage);
};

// void joinChannel(std::string msg, int clientFd, Server *_server);
// void handleMessage(std::string msg, int clientFd, Server *_server);
