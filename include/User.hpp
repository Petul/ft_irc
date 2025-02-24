/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   User.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 12:56:56 by pleander          #+#    #+#             */
/*   Updated: 2025/02/22 21:15:53 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class User
{
   public:
	User();
	User(int sockfd);

	int receiveData();
	int sendData(const std::string& buf);
	int getNextMessage(std::string& buf);

	const std::string& getPassword() const;
	void setPassword(std::string& pass);

	const std::string& getNick() const;
	void setNick(std::string& nick);

	const std::string& getUsername() const;
	void setUsername(std::string& username);

	const std::string& getRealname() const;
	void setRealName(std::string& realname);

	void markUserForDeletion();
	int getSocket() const;
	const std::string& getHost() const;

	void registerUser();
	bool isRegistered();

	void setIsOperator();
	bool getIsOperator();

	void applyUserMode(User& setter, const std::string& modes,
					   const std::string& param);

   private:
	int sockfd_;
	std::string recv_buf_;
	// std::string send_buf_;
	bool registered_;
	std::string password_;
	std::string username_;
	std::string realname_;
	std::string nick_;
	std::string host_;
	bool isOperator_;
};
