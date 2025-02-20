#include "Channel.hpp"
#include "replies.hpp"

Channel::Channel(std::string name, User &usr) : _name(name), _isInviteOnly(false),
												   _restrictionsOnTopic(false), _topic(""),
												   _password(""), _userLimit(MAX_USERS)
{
	_users.insert(&usr);
	_operators.insert(&usr);
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
		if (_users.find(&usr) == _users.end()) {
			_users.insert(&usr);
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

bool Channel::isUserInChannel(User &usr) {
	auto it = _users.find(&usr);
	if (it != _users.end())
		return true;
	return false;
}

bool Channel::isUserAnOperatorInChannel(User &usr) {
	auto it = _operators.find(&usr);
	if (it != _operators.end())
		return true;
	return false;
}

void Channel::displayMessage(User &sender, std::string msg) {
	for (auto user : _users) {
		if (sender.getSocket() != user->getSocket()) {
			std::string fullMsg = rplPrivMsg(sender.getNick(), _name, msg);
			write (user->getSocket(), fullMsg.c_str(), fullMsg.length());
		}
	}
}

void Channel::setInviteOnly()
{
	_isInviteOnly = true;
}
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

void Channel::addOperator(User &user) { _operators.insert(&user); }
void Channel::removeOperator(User &user) { _operators.erase(&user); }

void Channel::removeUser(User &source, std::string targetUsername, std::string reason)
{
	for (auto user : _users)
	{
		if (source.getSocket() != user->getSocket())
		{
			std::string fullMsg = rplKick(source.getNick(), _name, targetUsername, reason);
			write (user->getSocket(), fullMsg.c_str(), fullMsg.length());
		}
		if (user->getUsername() == targetUsername)
			_users.erase(user);
	}
}

void Channel::inviteUser(User &invitingUsr, std::unordered_map<int, User> &users_, std::string invitedUsrNickname) {
	for (auto itU = users_.begin(); itU != users_.end(); itU++)
	{
		if (itU->second.getNick() == invitedUsrNickname)
		{
			if (this->isUserAnOperatorInChannel(invitingUsr))
			{
				_invitedUsers.insert(&itU->second);
				std::string fullMsg = rplInviting("ourserver", invitingUsr.getNick(), itU->second.getNick(), _name);
				write (itU->second.getSocket(), fullMsg.c_str(), fullMsg.length());
			}
		}
	}
}

bool Channel::checkIfUserInvited(User &user) {
	auto it = _invitedUsers.find(&user);
	if (it == _invitedUsers.end())
		return false;
	return true;
}