#pragma once

#include <unistd.h>

#include <cstddef>
#include <iostream>
#include <istream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "Logger.hpp"
#include "User.hpp"

#define MAX_USERS 100  // to check

class Channel
{
   private:
	std::string _name;
	std::set<User*> _users;
	std::set<User*> _operators;
	std::set<User*> _invitedUsers;
	std::set<std::string> _banList;
	bool _isInviteOnly;
	bool _restrictionsOnTopic;
	std::string _topic;
	std::string _password;
	size_t _userLimit;

   public:
	Channel(std::string name, User& usr);
	~Channel();
	void shutDownChannel();

	std::string getName() const;
	bool getInviteMode() const;
	std::string getTopic() const;
	unsigned int getUserCount() const;
	std::set<User*> getUsers() const;

	void setInviteOnly();
	void unsetInviteOnly();

	void setUserLimit(int limit);
	void unsetUserLimit();

	void setPassword(std::string password);
	void unsetPasword();

	void setRestrictionsOnTopic();
	void unsetRestrictionsOnTopic();

	bool isUserAnOperatorInChannel(User& usr);
	bool isUserInChannel(User& usr);
	bool isUserInvited(User& usr);

	void addOperator(User& usr);
	void removeOperator(User& usr);
	void addUser(User& usr);
	void removeUser(User& usr);
	void removeAllUsers(std::string& msg);

	void broadcastToChannel(User& usr, const std::string& message);

	void displayChannelMessage(User& sender, std::string msg);

	void joinUser(const std::string& serverName, User& usr,
				  const std::string& attemptedPassword);

	void partUser(User& usr, const std::string& partMessage);

	void kickUser(User& usr, std::string targetUsername, std::string reason);

	void inviteUser(User& invitingUsr, std::unordered_map<int, User>& users_,
					std::string invitedUsrNickname);

	void showOrSetTopic(User& usr, std::string topic, int unsetTopicFlag);

	void applyChannelMode(User& setter, const std::string& modes,
						  const std::string& param);

	std::string getChannelModes() const;
	void printNames(User& usr);
	static const std::string avail_channel_modes;
};
