/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 15:58:57 by pleander          #+#    #+#             */
/*   Updated: 2025/02/22 21:02:07 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <map>

enum COMMANDTYPE
{
	NONE,
	PASS,
	NICK,
	USER,
	OPER,
	PRIVMSG,
	JOIN,
	PART,
	INVITE,
	WHO,
	WHOIS,
	QUIT,
	MODE,
	KICK,
	NOTICE,
	TOPIC,
	PING,
	PONG
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
	static const std::map<std::string, COMMANDTYPE> command_map_;
};
