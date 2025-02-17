/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:26:34 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 10:44:51 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// class Channel;  // Temp

#define PASS_MIN_LEN 4

#include <map>
#include <string>

#include "Message.hpp"
#include "User.hpp"

class Server
{
   public:
	Server();
	Server(std::string server_pass, int server_port);
	Server(const Server&);
	Server& operator=(const Server&);

	void startServer();

   private:
	std::string server_pass_;
	int server_port_;
	// std::map<std::string, Channel> channels_;
	std::map<int, User> users_;
};
