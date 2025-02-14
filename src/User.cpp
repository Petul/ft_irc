/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 13:32:51 by pleander          #+#    #+#             */
/*   Updated: 2025/02/14 13:38:29 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "User.hpp"

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
