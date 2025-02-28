/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmakkone <jmakkone@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 03:51:40 by jmakkone          #+#    #+#             */
/*   Updated: 2025/02/28 04:36:23 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include "GuardedChannel.hpp"

#define BUFFER_SIZE 509

class Bot
{
	public:
		Bot(const std::string& hostname, int port, const std::string& password);
		~Bot();

		void run();

	private:
		std::string hostname_;
		int port_;
		std::string password_;
		int sockfd_;

		std::vector<GuardedChannel> guardedChannels_;

		void sendCommand(const std::string& message);
		std::string parseTarget(const std::string& message,
				const std::string& tStart,
				const std::string& tEnd);
		void handleInput(const std::string& inputMsg);
		void closeConnection();
};
