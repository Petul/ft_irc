/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pleander <pleander@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/15 15:58:57 by pleander          #+#    #+#             */
/*   Updated: 2025/02/15 16:12:07 by pleander         ###   ########.fr       */
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
