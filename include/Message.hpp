/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 15:58:57 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 13:35:25 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>
#include <string>
#include <vector>

enum COMMANDTYPE
{
	NONE,
	PASS,
	NICK,
	USER,
	// Add more

};

// https://www.rfc-editor.org/rfc/rfc2812.html#section-2.3
// Server does not handle server-to-server communication and thus prefix is not
// supported
class Message
{
   public:
	Message(std::string& raw_msg);

	void parseMessage();
	COMMANDTYPE getType();
	std::vector<std::string>& getArgs();

   private:
	void parseType();
	std::string raw_message_;
	std::stringstream msg_ss_;
	COMMANDTYPE cmd_type_;
	std::vector<std::string> args_;
};
