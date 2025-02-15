/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 12:56:56 by pleander          #+#    #+#             */
/*   Updated: 2025/02/14 15:43:33 by pleander         ###   ########.fr       */
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
	void setPassword(std::string& pass);
	void setNick(std::string& nick);
	void setUsername(std::string& username);
	bool registerUser();

   private:
	int sockfd_;
	bool registered_{false};
	std::string password_;
	std::string username_;
	std::string nick_;
};
