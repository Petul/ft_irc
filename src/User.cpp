/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 13:32:51 by pleander          #+#    #+#             */
/*   Updated: 2025/02/14 15:26:37 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

#include <unistd.h>

#include <stdexcept>

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
	int n_bytes = recv(this->sockfd_, (char*)buf.data(), buf.size(), 0);
	if (n_bytes > 0)
	{
		return (1);
	}
	else if (n_bytes == 0)
	{
		close(this->sockfd_);
		return (0);
	}
	else
	{
		throw std::runtime_error{"Error: recv"};
	}
}
