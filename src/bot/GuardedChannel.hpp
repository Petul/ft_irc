/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GuardedChannel.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmakkone <jmakkone@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 03:52:49 by jmakkone          #+#    #+#             */
/*   Updated: 2025/02/28 03:55:48 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class GuardedChannel
{
	public:
		GuardedChannel(const std::string& owner,
				const std::string& channel,
				const std::string& password);
		~GuardedChannel();

		const std::string& getOwner() const;
		const std::string& getChannel() const;
		const std::string& getPassword() const;

	private:
		GuardedChannel();

		std::string owner_;
		std::string channel_;
		std::string password_;
};
