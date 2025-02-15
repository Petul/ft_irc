/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 18:16:37 by pleander          #+#    #+#             */
/*   Updated: 2025/02/15 18:55:40 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <chrono>
#include <string>

class Logger
{
   public:
	enum LEVEL
	{
		DEBUG,
		INFO,
		WARNING,
		ERROR
	};
	static void log(enum LEVEL lvl, std::string &msg);
	static const std::chrono::system_clock::time_point start_time_;
};
