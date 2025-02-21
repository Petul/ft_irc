/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 13:32:51 by pleander          #+#    #+#             */
/*   Updated: 2025/02/21 00:06:36 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

#include <unistd.h>

#include <algorithm>
#include <stdexcept>
#include <string>

#include "Logger.hpp"
#include "sys/socket.h"
#include <arpa/inet.h>
#include <netinet/in.h>

User::User() : sockfd_{-1}
{
}

User::User(int sockfd) : sockfd_{sockfd}, registered_{false}
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	if (getpeername(sockfd_, (struct sockaddr*)&addr, &addr_len) == 0)
	{
		host_ = inet_ntoa(addr.sin_addr);
	}
	else
	{
		host_ = "unknown";
	}
}


User::User(const User& o)
    : sockfd_{o.sockfd_},
      recv_buf_{o.recv_buf_},
      registered_{o.registered_},
      password_{o.password_},
      username_{o.username_},
      nick_{o.nick_},
	  host_{o.host_}
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
	this->recv_buf_ = o.recv_buf_;
	this->password_ = o.password_;
	this->registered_ = o.registered_;
	this->host_ = o.host_;
	return (*this);
}

/**
 * @brief Receives data from the user's socket. Throws an error on failure.
 *
 * @param buf: buffer to write data to
 * @return 1 if read successful, 0 if socket has been closed
 */
int User::receiveData()
{
	std::string buf(1024, 0);
	int n_bytes =
	    recv(this->sockfd_, const_cast<char*>(buf.data()), buf.size(), 0);
	// Client closed the connection
	if (n_bytes == 0)
	{
		close(this->sockfd_);
		return (0);
	}
	if (n_bytes < 0)
	{
		throw std::runtime_error{"receiveData"};
	}
	buf = buf.substr(0, n_bytes);  // Truncate buf to size of data
	Logger::log(Logger::DEBUG, "Received data from user " +
	                               std::to_string(sockfd_) + ": " + buf);
	// Check line break format
	if (buf.compare(n_bytes - 2, 2, "\r\n") != 0)
	{
		throw std::invalid_argument{
		    "Incorrect line break format, CRLF required"};
	}
	recv_buf_ += buf;
	return (1);
}

/**
 * @brief Gets the next CRLF separated message from the recv stream and stores
 * it in buf. Returns 0 if the recv stream is empty
 *
 * @param buf buffer to store message in
 * @return 0 if recv stream is emtpy
 */
int User::getNextMessage(std::string& buf)
{
	std::string part;
	if (recv_buf_.size() == 0)
	{
		return (0);
	}
	size_t pos = recv_buf_.find("\r\n", 0);
	buf = recv_buf_.substr(0, pos);
	recv_buf_.erase(recv_buf_.begin(), recv_buf_.begin() + pos + 2);
	return (1);
}

/**
 * @brief Sends the data in the buffer to the user
 *
 * @param buf: data to send
 * @return bytes written or throw error
 */
int User::sendData(const std::string& buf)
{
	//::send(this->sockfd_, buf.c_str(), buf.length(), 0);
	int n_bytes =
	    write(this->sockfd_, buf.c_str(), buf.length());
	    //write(this->sockfd_, const_cast<char*>(buf.data()), buf.size());
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

std::string& User::getHost()
{
    return (host_);
}
