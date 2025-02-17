/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 08:06:20 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 11:56:38 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

#include <stdexcept>
#include <string>

Message::Message(std::string &raw_message)
    : raw_message_{raw_message}, msg_ss_{raw_message}
{
	parseMessage();
}

void Message::parseMessage()
{
	if (raw_message_[0] == ':')
	{
		throw std::invalid_argument{"Server does not support prefix"};
	}
	parseType();
	while (!msg_ss_.eof())
	{
		std::string arg;
		msg_ss_ >> arg;
		if (arg[0] == ':')
		{
			while (!msg_ss_.eof())
			{
				arg.push_back(' ');
				msg_ss_ >> arg;
			}
		}
		args_.push_back(arg);
	}
}

void Message::parseType()
{
	std::string command;
	msg_ss_ >> command;
	if (command == "PASS ")
	{
		cmd_type_ = PASS;
	}
	else if (command == "NICK ")
	{
		cmd_type_ = NICK;
	}
	else if (command == "USER ")
	{
		cmd_type_ = USER;
	}
	else
	{
		throw std::invalid_argument("Invalid command: " + command);
	}
}
