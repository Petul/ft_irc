/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 13:32:51 by pleander          #+#    #+#             */
/*   Updated: 2025/02/18 02:17:51 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

#include <unistd.h>

#include <stdexcept>

#include "Logger.hpp"
#include "sys/socket.h"

User::User() : sockfd_{-1}
{
}

User::User(int sockfd) : sockfd_{sockfd}
{
}

User::User(const User& o)
    : sockfd_{o.sockfd_}, username_{o.username_}, nick_{o.nick_}
{
}

User& User::operator=(const User& o)
{
	if (this == &o)
	{
		return (*this);
	}
	this->sockfd_ = o.sockfd_;
	this->username_ = o.username_;
	this->nick_ = o.nick_;
	return (*this);
}

/**
 * @brief Receives data from the user's socket. Throws an error on failure.
 *
 * @param buf: buffer to write data to
 * @return 1 if read successful, 0 if socket has been closed
 */
int User::receiveData(std::string& buf)
{
	int n_bytes =
	    recv(this->sockfd_, const_cast<char*>(buf.data()), buf.size(), 0);
	if (n_bytes > 0)
	{
		return (1);
	}
	// Client closed the connection
	else if (n_bytes == 0)
	{
		close(this->sockfd_);
		return (0);
	}
	else
	{
		throw std::runtime_error{"Error: receiveData"};
	}
}

/**
 * @brief Sends the data in the buffer to the user
 *
 * @param buf: data to send
 * @return bytes written or throw error
 */
int User::sendData(std::string& buf)
{
	//::send(this->sockfd_, buf.c_str(), buf.length(), 0);
	int n_bytes =
	    write(this->sockfd_, const_cast<char*>(buf.data()), buf.size());
	if (n_bytes < 0)
	{
		throw std::runtime_error{"Error: sendData"};
	}
	return (n_bytes);
}

void User::setPassword(std::string& s)
{
	password_ = s;
}

void User::registerUser()
{
	Logger::log(Logger::DEBUG, "Registered user " + username_);
	registered_ = true;
}

bool User::isRegistered()
{
	return (registered_);
}

std::string& User::getPassword()
{
	return (password_);
}

void User::setNick(std::string& nick)
{
	nick_ = nick;
}

std::string& User::getNick()
{
	return (nick_);
}

void User::setUsername(std::string& username)
{
	username_ = username;
}

std::string& User::getUsername()
{
	return (username_);
}

int User::getSocket()
{
	return (sockfd_);
}
