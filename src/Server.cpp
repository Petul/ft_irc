/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 08:39:33 by pleander          #+#    #+#             */
/*   Updated: 2025/02/14 09:11:52 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

#include "sys/socket.h"

Server::Server() : Server("default", 8123)
{
}

Server::Server(std::string server_pass, int server_port)
    : server_pass_{server_pass}, server_port_{server_port}
{
}

Server::Server(const Server& o) : Server(o.server_pass_, o.server_port_)
{
}

Server& Server::operator=(const Server& o)
{
	if (this == &o)
	{
		return (*this);
	}
	this->server_port_ = o.server_port_;
	this->server_pass_ = o.server_pass_;
	return (*this);
}

void Server::startServer()
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
}
