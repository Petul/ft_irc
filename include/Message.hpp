/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 15:58:57 by pleander          #+#    #+#             */
/*   Updated: 2025/02/17 15:46:07 by mpellegr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>

enum COMMANDTYPE
{
	NONE,
	PASS,
	NICK,
	USER,
	JOIN,
	PRIVMSG
	// Add more

};

// class Message
// {
//    public:
// 	Message();
//
//       parseMesasge()
//
//    private:
// 	COMMAND cmd;
// 	std::vector<std::string> args;
// };
