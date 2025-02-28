/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GuardedChannel.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmakkone <jmakkone@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 03:56:13 by jmakkone          #+#    #+#             */
/*   Updated: 2025/02/28 03:56:48 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GuardedChannel.hpp"

GuardedChannel::GuardedChannel(const std::string& owner,
                               const std::string& channel,
                               const std::string& password)
    : owner_(owner), channel_(channel), password_(password)
{
}

GuardedChannel::~GuardedChannel()
{
}

const std::string& GuardedChannel::getOwner() const
{
    return owner_;
}

const std::string& GuardedChannel::getChannel() const
{
    return channel_;
}

const std::string& GuardedChannel::getPassword() const
{
    return password_;
}
