/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/06 13:22:08 by pleander          #+#    #+#             */
/*   Updated: 2025/03/06 13:22:18 by pleander         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

#include <chrono>
#include <ctime>
#include <iostream>

#define GREEN "\x1b[1;32m"
#define CYAN "\x1b[1;36m"
#define YELLOW "\x1b[1;33m"
#define RED "\x1b[1;31m"
#define RESET "\x1b[0m"

#ifndef LOG_LEVEL
#define LOG_LEVEL Logger::INFO
#endif

const std::chrono::system_clock::time_point Logger::start_time_{
	std::chrono::system_clock::now()};

void Logger::log(enum LEVEL lvl, std::string msg)
{
	if (lvl < LOG_LEVEL)
	{
		return;
	}
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now() - start_time_)
					.count();
	if (lvl == DEBUG) std::cout << GREEN << diff << " [DEBUG] ";
	if (lvl == INFO) std::cout << CYAN << diff << " [INFO] ";
	if (lvl == WARNING) std::cout << YELLOW << diff << " [WARNING] ";
	if (lvl == ERROR) std::cout << RED << diff << " [ERROR] ";

	cleanMessage(msg);
	std::cout << msg << RESET << std::endl;
}

void Logger::cleanMessage(std::string &msg)
{
	while (msg.back() == '\n' || msg.back() == '\r')
	{
		msg.pop_back();
	}
}
