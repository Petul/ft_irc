/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmakkone <jmakkone@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 03:54:17 by jmakkone          #+#    #+#             */
/*   Updated: 2025/02/28 03:54:43 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>
#include <cstdlib>

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        std::cout << "Usage: ./bot hostname port password\n";
        return 1;
    }
    try 
    {
        int port = std::stoi(argv[2]);
        Bot bot(argv[1], port, argv[3]);
        bot.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
