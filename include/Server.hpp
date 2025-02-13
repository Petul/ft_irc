/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/13 16:26:34 by pleander          #+#    #+#             */
/*   Updated: 2025/02/13 16:28:41 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

class Channel;  // Temp
class User;     // Temp

#include <map>
#include <string>
#include <vector>

class Server
{
   public:
	Server();
	Server(std::string server_pass, int server_port);
	Server(const Server&);
	Server& operator=(const Server&);

	void startServer();

   private:
	int server_port;
	std::string server_pass_;
	std::map<std::string, Channel> channels_;
	std::vector<User> users_;
};
