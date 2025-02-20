/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 08:06:20 by pleander          #+#    #+#             */
/*   Updated: 2025/02/20 10:43:07 by mpellegr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

#include "Logger.hpp"

Message::Message(std::string& raw_message)
    : raw_message_{raw_message}, msg_ss_{raw_message}
{
}

const std::map<std::string, COMMANDTYPE> Message::command_map_ = {
	{"PASS", PASS},
	{"NICK", NICK},
	{"USER", USER},
	{"OPER", OPER},
	{"PRIVMSG", PRIVMSG},
	{"JOIN", JOIN},
	{"PART", PART},
	{"INVITE", INVITE},
	{"WHO", WHO},
	{"QUIT", QUIT},
	{"MODE", MODE},
	{"KICK", KICK},
	{"NOTICE", NOTICE},
	{"TOPIC", TOPIC},
	{"PING", PING},
	{"PONG", PONG}
};

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
			// while (!msg_ss_.eof())
			// {
			// 	arg.push_back(' ');
			// 	msg_ss_ >> arg;
			// }
			std::string remaining;
			std::getline(msg_ss_, remaining);
			arg = arg.substr(1) + remaining;
		}
		if (arg[0])
		{
			args_.push_back(arg);
		}
	}
}

void Message::parseType()
{
	std::string command;
	msg_ss_ >> command;
	auto it = command_map_.find(command);
	if (it != command_map_.end())
	{
		cmd_type_ = it->second;
	} 
	else
	{
		throw std::invalid_argument("Invalid command: " + command);
	}
}

COMMANDTYPE Message::getType()
{
	return (cmd_type_);
}

std::vector<std::string>& Message::getArgs()
{
	return (args_);
}
