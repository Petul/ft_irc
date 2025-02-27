/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/14 13:30:16 by pleander          #+#    #+#             */
/*   Updated: 2025/02/20 13:19:42 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <cctype>
#include <stdexcept>

#include "Server.hpp"

inline void print_usage()
{
	std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		print_usage();
		return (0);
	}
	try
	{
		std::string p_str{argv[1]};
		for (auto it : p_str)
		{
			if (!isdigit(it))
			{
				throw std::invalid_argument{
					"Port must be numeric and positive"};
			}
		}
		int port = std::stoi(p_str);
		if (port < 1 || port > 65535)
		{
			throw std::invalid_argument{"Port must be in range 1-65535"};
		}
		std::string password{argv[2]};
		if (password.size() < 4)
		{
			throw std::invalid_argument{
				"Password must contain at least 4 characters"};
		}
		Server server(password, port);
		server.startServer();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		print_usage();
	}
	return (0);
}
