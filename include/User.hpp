/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 12:56:56 by pleander          #+#    #+#             */
/*   Updated: 2025/02/18 02:17:45 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class User
{
   public:
	User();
	User(int sockfd);
	User(const User&);
	User& operator=(const User&);

	int receiveData(std::string& buf);
	int sendData(std::string& buf);

	std::string& getPassword();
	void setPassword(std::string& pass);

	std::string& getNick();
	void setNick(std::string& nick);

	std::string& getUsername();
	void setUsername(std::string& username);

	int getSocket();

	void registerUser();
	bool isRegistered();

   private:
	int sockfd_;
	bool registered_{false};
	std::string password_;
	std::string username_;
	std::string nick_;
};
