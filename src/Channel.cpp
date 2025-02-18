#include "Channel.hpp"

Channel::Channel(std::string name, int cretorFd) : _name(name), _isInviteOnly(false),
												   _restrictionsOnTopic(false), _topic(""),
												   _password(""), _userLimit(MAX_USERS)
{
	_users.insert(cretorFd);
	_operators.insert(cretorFd);
}

Channel::~Channel() {}

std::string Channel::getName() const { return _name; }
bool Channel::getInviteMode() const { return _isInviteOnly; }
std::string Channel::getTopic() const { return _topic; }

void Channel::addUser(User &usr, std::string channelPassword) {
	int userFd = usr.getSocket();
	if (channelPassword != _password) {
		Logger::log(Logger::WARNING, "User " + usr.getUsername() + " used wrong/did't use password to join channel " + _name);
		return ;
	}
	if (_users.size() + 1 < _userLimit) {
		if (_users.find(userFd) == _users.end()) {
			_users.insert(userFd);
			std::string msg = "User " + std::to_string(userFd) + " joined " + _name;
			Logger::log(Logger::INFO, msg);
		}
		else {
			std::string msg = "User " + std::to_string(userFd) + " already in " + _name;
			Logger::log(Logger::INFO, msg);
		}
	} else {
		std::string msg = "Can't add user " + std::to_string(userFd) + " because reached user limit " + std::to_string(_userLimit);
		Logger::log(Logger::WARNING, msg);
	}
}

bool Channel::isUserInChannel(int userFd) {
	auto it = _users.find(userFd);
	if (it != _users.end()) 
		return true;
	return false;
}

bool Channel::isUserAnOperatorInChannel(int userFd) {
	auto it = _operators.find(userFd);
	if (it != _operators.end()) 
		return true;
	return false;
}

void Channel::displayMessage(int senderFd, std::string msg) {
	for (int userFd : _users) {
		if (senderFd != userFd) {
			std::string fullMsg = "[" + _name + "] User " + std::to_string(senderFd) + ": " + msg + "\n";
			write (userFd, fullMsg.c_str(), fullMsg.length());
		}
	}
}

void Channel::setInviteOnly() { _isInviteOnly = true; }
void Channel::unsetInviteOnly() { _isInviteOnly = false; }
void Channel::setUserLimit(int limit) {
	if (limit < MAX_USERS)
		_userLimit = limit;
}
void Channel::unsetUserLimit() { _userLimit = MAX_USERS; }
void Channel::setPassword(std::string password) { _password = password; }
void Channel::unsetPasword() { _password = ""; }
void Channel::setRestrictionsOnTopic() { _restrictionsOnTopic = true; }
void Channel::unsetRestrictionsOnTopic() { _restrictionsOnTopic = false; }

void Channel::addOperator(int userFd) { _operators.insert(userFd); }
void Channel::removeOperator(int userFd) { _operators.erase(userFd); }
void Channel::removeUser(int userFd) { _users.erase(userFd); }