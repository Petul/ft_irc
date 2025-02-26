#include "Channel.hpp"

#include "Server.hpp"
#include "replies.hpp"

Channel::Channel(std::string name, User& usr)
	: _name(name),
	  _isInviteOnly(false),
	  _restrictionsOnTopic(false),
	  _topic(""),
	  _password(""),
	  _userLimit(MAX_USERS)
{
	_users.insert(&usr);
	_operators.insert(&usr);
}

Channel::~Channel()
{
}

void Channel::shutDownChannel()
{
	std::string msg{"Channel shutting down"};
	removeAllUsers(msg);
}

void Channel::removeAllUsers(std::string& msg)
{
	std::set<User*> users_cpy{_users};
	for (auto usr : users_cpy)
	{
		partUser(*usr, msg);
	}
}

std::string Channel::getName() const
{
	return _name;
}

bool Channel::getInviteMode() const
{
	return _isInviteOnly;
}

std::string Channel::getTopic() const
{
	return _topic;
}

unsigned int Channel::getUserCount() const
{
	return _users.size();
}

std::set<User*> Channel::getUsers() const
{
	return _users;
}

void Channel::setInviteOnly()
{
	_isInviteOnly = true;
}

void Channel::unsetInviteOnly()
{
	_isInviteOnly = false;
}

void Channel::setUserLimit(int limit)
{
	if (limit < MAX_USERS)
	{
		_userLimit = limit;
	}
}

void Channel::unsetUserLimit()
{
	_userLimit = MAX_USERS;
}

void Channel::setPassword(std::string password)
{
	_password = password;
}

void Channel::unsetPasword()
{
	_password.clear();
}

void Channel::setRestrictionsOnTopic()
{
	_restrictionsOnTopic = true;
}

void Channel::unsetRestrictionsOnTopic()
{
	_restrictionsOnTopic = false;
}

bool Channel::isUserAnOperatorInChannel(User& usr)
{
	return _operators.find(&usr) != _operators.end();
}

bool Channel::isUserInChannel(User& usr)
{
	return _users.find(&usr) != _users.end();
}

bool Channel::isUserInvited(User& usr)
{
	return _invitedUsers.find(&usr) != _invitedUsers.end();
}

void Channel::addOperator(User& usr)
{
	_operators.insert(&usr);
}

void Channel::removeOperator(User& usr)
{
	_operators.erase(&usr);
}

void Channel::addUser(User& usr)
{
	int userFd = usr.getSocket();
	if (_users.find(&usr) == _users.end())
	{
		_users.insert(&usr);
		Logger::log(Logger::INFO,
					"User " + std::to_string(userFd) + " joined " + _name);
	}
	else
	{
		Logger::log(Logger::INFO, "User " + std::to_string(userFd) +
									  " is already in " + _name);
	}
}

void Channel::removeUser(User& usr)
{
	auto it = _users.find(&usr);
	if (it != _users.end()) _users.erase(it);
	// Do we remove users from these as well?
	//_operators.erase(user);
	//_invitedUsers.erase(user);
}

void Channel::broadcastToChannel(User& usr, const std::string& message)
{
	for (auto user : _users)
	{
		if (user == &usr) continue;
		user->sendData(message);
	}
}

void Channel::displayChannelMessage(User& sender, std::string msg)
{
	for (auto user : _users)
	{
		if (sender.getSocket() != user->getSocket())
		{
			std::string fullMsg = rplPrivMsg(sender.getNick(), _name, msg);
			user->sendData(fullMsg);
			//   write(user->getSocket(), fullMsg.c_str(), fullMsg.length());
		}
	}
}

void Channel::joinUser(const std::string& serverName, User& usr,
					   const std::string& attemptedPassword)
{
	if (_isInviteOnly && !isUserInvited(usr))
	{
		usr.sendData(errInviteOnlyChan(serverName, usr.getNick(), _name));
		return;
	}

	if (!_password.empty() && attemptedPassword != _password)
	{
		usr.sendData(errBadChannelKey(serverName, usr.getNick(), _name));
		Logger::log(Logger::WARNING,
					"User " + usr.getUsername() +
						" used wrong/did't use password to join channel " +
						_name);

		return;
	}

	if (getUserCount() >= _userLimit)
	{
		usr.sendData(errChannelIsFull(serverName, usr.getNick(), _name));
		std::string msg = "Can't add user " + std::to_string(usr.getSocket()) +
						  " because reached user limit " +
						  std::to_string(_userLimit);
		Logger::log(Logger::WARNING, msg);
		return;
	}

	addUser(usr);
	usr.incUsrChannelCount();
	std::string joinMsg =
		rplJoin(usr.getNick(), usr.getUsername(), usr.getHost(), _name);

	broadcastToChannel(usr, joinMsg);
	usr.sendData(joinMsg);

	if (_topic.empty())
	{
		usr.sendData(rplNoTopic(serverName, usr.getNick(), _name));
	}
	else
	{
		usr.sendData(rplTopic(serverName, usr.getNick(), _name, _topic));
	}

	std::string nameList;
	for (std::set<User*>::iterator it = _users.begin(); it != _users.end();
		 ++it)
	{
		User* u = *it;
		if (isUserAnOperatorInChannel(*u))
			nameList += "@" + u->getNick() + " ";
		else
			nameList += u->getNick() + " ";
	}

	// RPL_NAMREPLY(353)
	usr.sendData(rplNamReply(serverName, usr.getNick(), "=", _name, nameList));
	// RPL_ENDOFNAMES(366)
	usr.sendData(rplEndOfNames(serverName, usr.getNick(), _name));
}

void Channel::partUser(User& usr, const std::string& partMessage)
{
	std::string partMsg = rplPart(usr.getNick(), usr.getUsername(),
								  usr.getHost(), _name, partMessage);
	removeUser(usr);
	for (auto user : _users)
	{
		user->sendData(partMsg);
	}
	usr.decUsrChannelCount();
	usr.sendData(partMsg);
	Logger::log(Logger::INFO,
				"User " + usr.getNick() + " parted channel " + _name);
}

void Channel::kickUser(User& source, std::string targetUsername,
					   std::string reason)
{
	if (!isUserInChannel(source))
	{
		source.sendData(
			errUserNotInChannel(SERVER_NAME, source.getNick(), _name));
		return;
	}
	if (!isUserAnOperatorInChannel(source))
	{
		source.sendData(
			errChanPrivsNeeded(SERVER_NAME, source.getNick(), _name));
		return;
	}

	User* targetUser = nullptr;
	for (auto user : _users)
	{
		if (user->getNick() == targetUsername)
		{
			targetUser = user;
			break;
		}
	}

	if (!targetUser)
	{
		source.sendData(errNotOnChannel(SERVER_NAME, source.getNick(), _name));
		return;
	}

	std::string kickMsg =
		rplKick(source.getNick(), _name, targetUser->getNick(), reason);
	broadcastToChannel(*targetUser, kickMsg);
	targetUser->sendData(kickMsg);
	removeUser(*targetUser);
	Logger::log(Logger::INFO, "User " + targetUser->getNick() +
								  " was kicked from channel " + _name + " by " +
								  source.getNick());
}

void Channel::inviteUser(User& invitingUsr,
						 std::unordered_map<int, User>& users_,
						 std::string invitedUsrNickname)
{
	if (getInviteMode() && !this->isUserAnOperatorInChannel(invitingUsr))
	{
		invitingUsr.sendData(
			errChanPrivsNeeded(SERVER_NAME, invitingUsr.getNick(), _name));
		return;
	}

	for (auto itU = users_.begin(); itU != users_.end(); itU++)
	{
		if (itU->second.getNick() == invitedUsrNickname)
		{
			if (_invitedUsers.find(&itU->second) != _invitedUsers.end())
			{
				invitingUsr.sendData(
					errUserOnChannel(SERVER_NAME, invitingUsr.getNick(),
									 invitedUsrNickname, _name));
				return;
			}
			if (!itU->second.getAwayMsg().empty())
			{
				invitingUsr.sendData(rplAway(SERVER_NAME, invitingUsr.getNick(),
											 itU->second.getNick(),
											 itU->second.getAwayMsg()));
			}
			_invitedUsers.insert(&itU->second);
			std::string fullMsg =
				rplInviting("ourserver", invitingUsr.getNick(),
							itU->second.getNick(), _name);
			invitingUsr.sendData(fullMsg);
			itU->second.sendData(":" + invitingUsr.getNick() + " INVITE " +
								 invitedUsrNickname + " " + _name + "\r\n");
			// TODO: Hey we still need to send the invite!
			return;
		}
	}
	invitingUsr.sendData(
		errNoSuchNick(SERVER_NAME, invitingUsr.getNick(), invitedUsrNickname));
}

void Channel::showOrSetTopic(User& usr, std::string newTopic,
							 int unsetTopicFlag)
{
	if (!isUserInChannel(usr))
	{
		usr.sendData(errUserNotInChannel(SERVER_NAME, usr.getNick(), _name));
		return;
	}
	if (_restrictionsOnTopic == true)
	{
		if (!isUserAnOperatorInChannel(usr))
		{
			usr.sendData(errChanPrivsNeeded(SERVER_NAME, usr.getNick(), _name));
			return;
		}
	}
	if (unsetTopicFlag)
	{
		usr.sendData(rplTopic(SERVER_NAME, usr.getNick(), _name, _topic));
		Logger::log(Logger::INFO, "User " + usr.getNick() +
									  " removed topic from channel " + _name);
		_topic.clear();
	}
	else
	{
		if (newTopic.empty())
		{
			if (_topic.empty())
			{
				usr.sendData(rplNoTopic(SERVER_NAME, usr.getNick(), _name));
				Logger::log(Logger::INFO, "User " + usr.getNick() +
											  " requested topic from channel " +
											  _name + " but topic is not set");
			}
			else
			{
				usr.sendData(
					rplTopic(SERVER_NAME, usr.getNick(), _name, _topic));
				Logger::log(Logger::INFO, "User " + usr.getNick() +
											  " topic of channel " + _name +
											  " is " + _topic);
			}
		}
		else
		{
			_topic = newTopic;
			usr.sendData(rplTopic(SERVER_NAME, usr.getNick(), _name, _topic));
			broadcastToChannel(
				usr, rplTopic(SERVER_NAME, usr.getNick(), _name, _topic));
			Logger::log(Logger::INFO, "User " + usr.getNick() +
										  " changet topic in channel " + _name +
										  ", new topic is " + _topic);
		}
	}
}

void Channel::applyChannelMode(User& setter, const std::string& modes,
							   const std::string& param)
{
	bool adding = true;
	size_t i = 0;
	while (i < modes.size())
	{
		char c = modes[i];
		if (c == '+')
		{
			adding = true;
		}
		else if (c == '-')
		{
			adding = false;
		}
		else
		{
			switch (c)
			{
				case 'b':
					setter.sendData(
						rplEndOfBanList(SERVER_NAME, setter.getNick(), _name));
					break;
				case 'i':
					if (adding)
					{
						setInviteOnly();
						Logger::log(Logger::DEBUG,
									"user " + setter.getUsername() +
										" set invite only mode ON in channel " +
										_name);
					}
					else
					{
						unsetInviteOnly();
						Logger::log(
							Logger::DEBUG,
							"user " + setter.getUsername() +
								" set invite only mode OFF in channel " +
								_name);
					}
					break;
				case 't':
					if (adding)
					{
						setRestrictionsOnTopic();
						Logger::log(Logger::DEBUG,
									"user " + setter.getUsername() +
										" set restrictions on topic mode ON in "
										"channel " +
										_name);
					}
					else
					{
						unsetRestrictionsOnTopic();
						Logger::log(Logger::DEBUG,
									"user " + setter.getUsername() +
										" set restrictions on topic mode OFF "
										"in channel " +
										_name);
					}
					break;
				case 'k':
					if (adding)
					{
						setPassword(param);
						Logger::log(Logger::DEBUG,
									"user " + setter.getUsername() +
										" set password in channel " + _name);
					}
					else
					{
						unsetPasword();
						Logger::log(Logger::DEBUG,
									"user " + setter.getUsername() +
										" removed password in channel " +
										_name);
					}
					break;
				case 'l':
					if (adding)
					{
						setUserLimit(std::stoi(param));
						Logger::log(Logger::DEBUG, "User " +
													   setter.getUsername() +
													   "set channel limit to " +
													   param + " users");
					}
					else
					{
						unsetUserLimit();
						Logger::log(Logger::DEBUG, "User " +
													   setter.getUsername() +
													   "unset channel limit");
					}
					break;
				case 'o':
					if (!param.empty())
					{
						bool foundUser = false;
						for (auto user : _users)
						{
							User* targetUser = user;

							if (targetUser->getNick() == param)
							{
								foundUser = true;

								if (adding)
								{
									if (isUserInChannel(*targetUser))
									{
										addOperator(*targetUser);
										Logger::log(
											Logger::DEBUG,
											"User " + setter.getUsername() +
												" gave channel-operator "
												"privilege on " +
												_name + " to user " +
												targetUser->getUsername());
									}
									else
									{
										setter.sendData(errUserNotInChannel(
											SERVER_NAME, setter.getNick(),
											_name));
										Logger::log(
											Logger::DEBUG,
											"Tried to +o " +
												targetUser->getUsername() +
												" but they're not in channel " +
												_name);
									}
								}
								else
								{
									if (isUserAnOperatorInChannel(*targetUser))
									{
										removeOperator(*targetUser);
										Logger::log(
											Logger::DEBUG,
											"User " + setter.getUsername() +
												" revoked operator privilege "
												"on " +
												_name + " from user " +
												targetUser->getUsername());
									}
									else
									{
										Logger::log(
											Logger::DEBUG,
											"Tried to -o user " +
												targetUser->getUsername() +
												" who was not a channel "
												"operator in " +
												_name);
									}
								}
								break;
							}
						}
						if (!foundUser)
						{
							setter.sendData(errNoSuchNick(
								SERVER_NAME, setter.getNick(), param));
							Logger::log(
								Logger::DEBUG,
								"MODE +o: Could not find user with nick '" +
									param + "'");
						}
						break;
					}
					break;
				default:
					setter.sendData(errUnknownMode(SERVER_NAME,
												   setter.getNick(), _name,
												   std::string(1, c)));
					break;
			}
		}
		i++;
	}

	std::string modeChangeMsg =
		rplChannelMode(setter.getNick(), setter.getUsername(), setter.getHost(),
					   _name, modes, param);
	broadcastToChannel(setter, modeChangeMsg);
}

std::string Channel::getChannelModes() const
{
	std::string modes = "";
	if (_isInviteOnly) modes += "i";
	if (_restrictionsOnTopic) modes += "t";
	if (_userLimit != MAX_USERS) modes += "l";
	if (!_password.empty()) modes += "k";
	if (!modes.empty()) modes = "+" + modes;
	return modes;
}
