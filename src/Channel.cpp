#include "Channel.hpp"

Channel::Channel(std::string name, int cretorFd) : _name(name) {
	_users.insert(cretorFd);
	_operators.insert(cretorFd);
}

Channel::~Channel() {}

std::string Channel::getName() const { return _name; }

void Channel::addUser(int userFd) {
	if (_users.find(userFd) == _users.end()) {
		_users.insert(userFd);
		std::string msg = "User " + std::to_string(userFd) + " joined " + _name;
		Logger::log(Logger::INFO, msg);
	}
	else {
		std::string msg = "User " + std::to_string(userFd) + " already in " + _name;
		Logger::log(Logger::INFO, msg);
	}
}

bool Channel::isUserInChannel(int userFd) {
	auto it = _users.find(userFd);
	if (it != _users.end()) 
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