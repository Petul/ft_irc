/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmakkone <jmakkone@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 17:55:46 by jmakkone          #+#    #+#             */
/*   Updated: 2025/02/22 21:32:58 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>


//GENERIC REPLIES

// ERR_NEEDMOREPARAMS
inline std::string errNeedMoreParams(const std::string& serverName,
		const std::string& nick, const std::string& command)
{
	return ":" + serverName + " 461 " + nick + " " + command + " :Not enough parameters\r\n";
}

// ERR_ALREADYREGISTRED (462)
inline std::string errAlreadyRegistered()
{
	return "462  :Unauthorized command (already registered)\r\n";
}






//Welcome message which is sent after succeful user registertion

// 001 RPL_WELCOME
inline std::string rplWelcome(const std::string& serverName,
		const std::string& nick,
		const std::string& user,
		const std::string& host)
{
	// "Welcome to the Internet Relay Network <nick>!<user>@<host>"
	return ":" + serverName + " 001 " + nick + " :Welcome to the Internet Relay Network " +
		nick + "!~" + user + "@" + host + "\r\n";
}

// 002 RPL_YOURHOST
inline std::string rplYourHost(const std::string& serverName,
		const std::string& nick,
		const std::string& version)
{
	// "Your host is <servername>, running version <ver>"
	return ":" + serverName + " 002 " + nick + " :Your host is " + serverName +
		", running version " + version + "\r\n";
}

// 003 RPL_CREATED
inline std::string rplCreated(const std::string& serverName,
		const std::string& nick,
		const std::string& date)
{
	return ":" + serverName + " 003 " + nick + " :This server was created " + date + "\r\n";
}

// 004 RPL_MYINFO
inline std::string rplMyInfo(const std::string& serverName,
		const std::string& nick,
		const std::string& version,
		const std::string& userModes,
		const std::string& channelModes)
{
	// "<servername> <version> <available user modes> <available channel modes>"
	return ":" + serverName + " 004 " + nick + " :" + serverName + " " + version + " " +
		userModes + " " + channelModes + "\r\n";
}






// NICK Command Replies/Errors

// When a user changes their nick, broadcast the change:
inline std::string rplNickChange(const std::string& oldNick,
		const std::string& user,
		const std::string& host,
		const std::string& newNick)
{
	return ":" + oldNick + "!" + user + "@" + host + " NICK :" + newNick + "\r\n";
}

// ERR_NICKNAMEINUSE (433)
inline std::string errNicknameInUse(const std::string& serverName,
		const std::string& nick)
{
	return ":" + serverName + " 433 " + nick + " " + nick + " :Nickname is already in use\r\n";
}

// ERR_NONICKNAMEGIVEN (431)
inline std::string errNoNicknameGiven(const std::string& serverName,
		const std::string& nick)
{
	return ":" + serverName + " 431 " + nick + " :No nickname given\r\n";
}

// ERR_RESTRICTED (484)
// "Your connection is restricted"
inline std::string errRestricted(const std::string& serverName,
                                 const std::string& nick)
{
    return ":" + serverName + " 484 " + nick + " :Your connection is restricted\r\n";
}

// ERR_ERRONEUSNICKNAME (432)
// "Erroneus nickname"
inline std::string errErroneusNickname(const std::string& serverName,
                                       const std::string& nick)
{
    return ":" + serverName + " 432 " + nick + " :Erroneous nickname\r\n";
}

// ERR_NICKCOLLISION (436)
// "Nickname collision"
inline std::string errNickCollision(const std::string& serverName,
                                    const std::string& nick,
                                    const std::string& collisionNick)
{
    return ":" + serverName + " 436 " + nick + " " + collisionNick + " :Nickname collision\r\n";
}






// OPER Command Replies/Errors

// RPL_YOUREOPER (381)
inline std::string rplYoureOper(const std::string& serverName,
		const std::string& nick)
{
	return ":" + serverName + " 381 " + nick + " :You are now an IRC operator\r\n";
}

// ERR_NOOPERHOST (491) -- if operator login fails due to host restrictions
inline std::string errNoOperHost(const std::string& serverName,
		const std::string& nick)
{
	return ":" + serverName + " 491 " + nick + " :No O-lines for your host\r\n";
}

// ERR_PASSWDMISMATCH (464)
// "Password mismatch"
inline std::string errPasswdMismatch(const std::string& serverName,
                                     const std::string& nick)
{
    return ":" + serverName + " 464 " + nick + " :Password mismatch\r\n";
}





// QUIT Command Broadcast

inline std::string rplQuit(const std::string& nick,
		const std::string& username,
		const std::string& hostname,
		const std::string& message)
{
	return ":" + nick + "!~" + username + "@" + hostname + " QUIT :" + message + "\r\n";
}







// JOIN Command Replies/Errors

// Broadcast JOIN message
inline std::string rplJoin(const std::string& nick,
		const std::string& username,
		const std::string& hostname,
		const std::string& channel)
{
	return ":" + nick + "!~" + username + "@" + hostname + " JOIN " + channel + "\r\n";
}

// RPL_NAMEREPLY 353 <nick> <symbol> <channel> :<names>
inline std::string rplNamReply(const std::string& serverName,
		const std::string& nick,
		const std::string& symbol,
		const std::string& channel,
		const std::string& namelist)
{
	return ":" + serverName + " 353 " + nick + " " + symbol + " " +
		channel + " :" + namelist + "\r\n";
}

// RPL_ENDOFNAMES (366) - after enumerating all users
inline std::string rplEndOfNames(const std::string& serverName,
		const std::string& nick,
		const std::string& channel)
{
	return ":" + serverName + " 366 " + nick + " " + channel +
		" :End of /NAMES list\r\n";
}

// "Cannot join channel (+b)"
inline std::string errBannedFromChan(const std::string& serverName,
                                     const std::string& nick,
                                     const std::string& channel)
{
    return ":" + serverName + " 474 " + nick + " " + channel + " :Cannot join channel (+b)\r\n";
}

// ERR_INVITEONLYCHAN (473)
// "Cannot join channel (+i)"
inline std::string errInviteOnlyChan(const std::string& serverName,
                                       const std::string& nick,
                                       const std::string& channel)
{
    return ":" + serverName + " 473 " + nick + " " + channel + " :Cannot join channel (+i)\r\n";
}

// ERR_BADCHANNELKEY (475)
// "Cannot join channel (+k)"
inline std::string errBadChannelKey(const std::string& serverName,
                                    const std::string& nick,
                                    const std::string& channel)
{
    return ":" + serverName + " 475 " + nick + " " + channel + " :Cannot join channel (+k)\r\n";
}

// ERR_CHANNELISFULL (471)
// "Cannot join channel (+l)"
inline std::string errChannelIsFull(const std::string& serverName,
                                    const std::string& nick,
                                    const std::string& channel)
{
    return ":" + serverName + " 471 " + nick + " " + channel + " :Cannot join channel (+l)\r\n";
}

// ERR_BADCHANMASK (476)
// "Bad Channel Mask"
inline std::string errBadChanMask(const std::string& serverName,
                                  const std::string& nick,
                                  const std::string& channel)
{
    return ":" + serverName + " 476 " + nick + " " + channel + " :Bad Channel Mask\r\n";
}

// ERR_NOSUCHCHANNEL (403)
// "No such channel"
inline std::string errNoSuchChannel(const std::string& serverName,
                                    const std::string& nick,
                                    const std::string& channel)
{
    return ":" + serverName + " 403 " + nick + " " + channel + " :No such channel\r\n";
}

// ERR_TOOMANYCHANNELS (405)
// "You have joined too many channels"
inline std::string errTooManyChannels(const std::string& serverName,
                                      const std::string& nick)
{
    return ":" + serverName + " 405 " + nick + " :You have joined too many channels\r\n";
}

// ERR_TOOMANYTARGETS (407)
// "Duplicate recipients. No message delivered"
inline std::string errTooManyTargets(const std::string& serverName,
                                     const std::string& nick)
{
    return ":" + serverName + " 407 " + nick + " :Duplicate recipients. No message delivered\r\n";
}

// ERR_UNAVAILRESOURCE (437)
// "Resource temporarily unavailable"
inline std::string errUnavailableResource(const std::string& serverName,
                                          const std::string& nick)
{
    return ":" + serverName + " 437 " + nick + " :Resource temporarily unavailable\r\n";
}








// PART Command Replies

// Broadcast PART message
inline std::string rplPart(const std::string &nick,
                           const std::string &username,
                           const std::string &hostname,
                           const std::string &channelName,
                           const std::string &partMessage)
{
    return ":" + nick + "!~" + username + "@" + hostname + " PART " + channelName + " :" + partMessage + "\r\n";
}







// MODE Command Replies/Errors

// Broadcast MODE change message
inline std::string rplChannelMode(const std::string& nick,
		const std::string& username,
		const std::string& hostname,
		const std::string& channelName,
		const std::string& modes,
		const std::string& params)
{
	return ":" + nick + "!~" + username + "@" + hostname + " MODE " + channelName + " " + modes + " " + params + "\r\n";
}

// ERR_KEYSET (467)
// "Channel key already set"
inline std::string errKeySet(const std::string& serverName,
                               const std::string& nick,
                               const std::string& channel)
{
    return ":" + serverName + " 467 " + nick + " " + channel + " :Channel key already set\r\n";
}

// ERR_NOCHANMODES (477) - custom or non-standard; indicates no channel modes are supported
inline std::string errNoChanModes(const std::string& serverName,
                                  const std::string& nick,
                                  const std::string& channel)
{
    return ":" + serverName + " 477 " + nick + " " + channel + " :Channel doesn't support modes\r\n";
}

// ERR_CHANOPRIVSNEEDED (482)
// "You're not channel operator"
inline std::string errChanPrivsNeeded(const std::string& serverName,
                                      const std::string& nick,
                                      const std::string& channel)
{
    return ":" + serverName + " 482 " + nick + " " + channel + " :You're not channel operator\r\n";
}

// ERR_USERNOTINCHANNEL (441)
// "They aren't on that channel"
inline std::string errUserNotInChannel(const std::string& serverName,
                                       const std::string& nick,
                                       const std::string& channel)
{
    return ":" + serverName + " 441 " + nick + " " + channel + " :They aren't on that channel\r\n";
}

// ERR_UNKNOWNMODE (472)
// "Unknown mode"
inline std::string errUnknownMode(const std::string& servername,
                                  const std::string& nick,
                                  const std::string& channel,
                                  const std::string& mode)
{
    return ":" + servername + " 472 " + nick + " " + channel + " :unknown mode " + mode + "\r\n";
}

// ERR_UMODEUNKNOWNFLAG (501)
// ":Unknown MODE flag"
inline std::string errUnknownModeFlag(const std::string& servername,
                                  const std::string& nick)
{
    return ":" + servername + " 501 " + nick + " :unknown MODE flag" + "\r\n";
}

// RPL_CHANNELMODEIS (324)
// "<channel> <modes> [<mode params>]"
inline std::string rplChannelModeIs(const std::string& serverName,
                                    const std::string& nick,
                                    const std::string& channel,
                                    const std::string& modes)
{
    return ":" + serverName + " 324 " + nick + " " + channel + " " + modes + "\r\n";
}

// RPL_BANLIST (367)
// "<channel> <ban mask> <set by> <time>"
inline std::string rplBanList(const std::string& serverName,
                              const std::string& nick,
                              const std::string& channel,
                              const std::string& banMask,
                              const std::string& setBy,
                              const std::string& timeStamp)
{
    return ":" + serverName + " 367 " + nick + " " + channel + " " + banMask + " " + setBy + " " + timeStamp + "\r\n";
}

// RPL_ENDOFBANLIST (368)
// "<channel> :End of ban list"
inline std::string rplEndOfBanList(const std::string& serverName,
                                   const std::string& nick,
                                   const std::string& channel)
{
    return ":" + serverName + " 368 " + nick + " " + channel + " :End of ban list\r\n";
}

// RPL_EXCEPTLIST (348)
// "<channel> <exception mask>"
inline std::string rplExceptList(const std::string& serverName,
                                 const std::string& nick,
                                 const std::string& channel,
                                 const std::string& exceptionMask)
{
    return ":" + serverName + " 348 " + nick + " " + channel + " " + exceptionMask + "\r\n";
}

// RPL_ENDOFEXCEPTLIST (349)
// "<channel> :End of exception list"
inline std::string rplEndOfExceptList(const std::string& serverName,
                                      const std::string& nick,
                                      const std::string& channel)
{
    return ":" + serverName + " 349 " + nick + " " + channel + " :End of exception list\r\n";
}

// RPL_INVITELIST (346)
// "<channel> <invite mask>"
inline std::string rplInviteList(const std::string& serverName,
                                 const std::string& nick,
                                 const std::string& channel,
                                 const std::string& inviteMask)
{
    return ":" + serverName + " 346 " + nick + " " + channel + " " + inviteMask + "\r\n";
}

// RPL_ENDOFINVITELIST (347)
// "<channel> :End of invite list"
inline std::string rplEndOfInviteList(const std::string& serverName,
                                      const std::string& nick,
                                      const std::string& channel)
{
    return ":" + serverName + " 347 " + nick + " " + channel + " :End of invite list\r\n";
}

// RPL_UNIQOPIS (325)
// "<channel> :<operator list>"
inline std::string rplUniqOpIs(const std::string& serverName,
                               const std::string& nick,
                               const std::string& channel,
                               const std::string& opList)
{
    return ":" + serverName + " 325 " + nick + " " + channel + " :" + opList + "\r\n";
}









// TOPIC Command Replies/Errors

// RPL_NOTOPIC (331)
inline std::string rplNoTopic(const std::string& serverName,
		const std::string& nick,
		const std::string& channel)
{
	return ":" + serverName + " 331 " + nick + " " + channel + " :No topic is set\r\n";
}

// RPL_TOPIC (332)
inline std::string rplTopic(const std::string& serverName,
		const std::string& nick,
		const std::string& channel,
		const std::string& topic)
{
	return ":" + serverName + " 332 " + nick + " " + channel + " :" + topic + "\r\n";
}









// INVITE Command Replies/Errors

// RPL_INVITING (341)
inline std::string rplInviting(const std::string& serverName,
		const std::string& nick,
		const std::string& target,
		const std::string& channel)
{
	return ":" + serverName + " 341 " + nick + " " + target + " " + channel + "\r\n";
}

// ERR_USERONCHANNEL (443)
// Format: ":<server> 443 <nick> <user> <channel> :is already on channel"
inline std::string errUserOnChannel(const std::string& serverName,
                                    const std::string& nick,
                                    const std::string& user,
                                    const std::string& channel)
{
    return ":" + serverName + " 443 " + nick + " " + user + " " + channel +
           " :is already on channel\r\n";
}





// KICK Command Replies/Errors

// ERR_NOTONCHANNEL (442)
inline std::string errNotOnChannel(const std::string& serverName,
		const std::string& nick,
		const std::string& channel)
{
	return ":" + serverName + " 442 " + nick + " " + channel + " :You're not on that channel\r\n";
}

// Broadcast KICK message
inline std::string rplKick(const std::string& source,
		const std::string& channel,
		const std::string& target,
		const std::string& reason)
{
	return ":" + source + " KICK " + channel + " " + target + " :" + reason + "\r\n";
}








// PRIVMSG/NOTICE Command Replies/Errors

// ERR_NORECIPIENT (411)
inline std::string errNoRecipient(const std::string& serverName,
		const std::string& nick,
		const std::string& command)
{
	return ":" + serverName + " 411 " + nick + " " + command + " :No recipient given\r\n";
}

// ERR_CANNOTSENDTOCHAN (404)
// "Cannot send to channel"
inline std::string errCannotSendToChan(const std::string& serverName,
                                       const std::string& nick,
                                       const std::string& channel)
{
    return ":" + serverName + " 404 " + nick + " " + channel + " :Cannot send to channel\r\n";
}

// ERR_WILDTOPLEVEL (414)
// "Wildcard not allowed in toplevel domain"
inline std::string errWildToplevel(const std::string& serverName,
                                   const std::string& nick,
                                   const std::string& mask)
{
    return ":" + serverName + " 414 " + nick + " " + mask + " :Wildcard not allowed in toplevel domain\r\n";
}

// ERR_NOSUCHNICK (401)
inline std::string errNoSuchNick(const std::string& serverName,
		const std::string& nick,
		const std::string& target)
{
	return ":" + serverName + " 401 " + nick + " " + target + " :No such nick/channel\r\n";
}

// RPL_AWAY (301)
// "<nick> <target> :<away message>"
inline std::string rplAway(const std::string& serverName,
                           const std::string& nick,
                           const std::string& target,
                           const std::string& awayMessage) {
    return ":" + serverName + " 301 " + nick + " " + target + " :" + awayMessage + "\r\n";
}

// RPL_UNAWAY (305)
// ":You are no longer marked as being away"
inline std::string rplUnaway(const std::string& serverName,
                               const std::string& nick) {
    return ":" + serverName + " 305 " + nick + " :You are no longer marked as being away\r\n";
}

// RPL_NOWAWAY (306)
// ":You have been marked as being away"
inline std::string rplNowAway(const std::string& serverName,
                              const std::string& nick) {
    return ":" + serverName + " 306 " + nick + " :You have been marked as being away\r\n";
}

// ERR_NOTEXTTOSEND (412)
// "No text to send"
inline std::string errNoTextToSend(const std::string& serverName,
                                   const std::string& nick) {
    return ":" + serverName + " 412 " + nick + " :No text to send\r\n";
}

// ERR_NOTOPLEVEL (413)
// "No toplevel domain specified"
inline std::string errNotToplevel(const std::string& serverName,
                                  const std::string& nick,
                                  const std::string& mask) {
    return ":" + serverName + " 413 " + nick + " " + mask + " :No toplevel domain specified\r\n";
}


//I'm not sure do we use these
//
// Standard PRIVMSG (for broadcast, not a reply code)
inline std::string rplPrivMsg(const std::string& source,
		const std::string& target,
		const std::string& message)
{
	return ":" + source + " PRIVMSG " + target + " :" + message + "\r\n";
}

// Standard NOTICE (for broadcast)
inline std::string rplNotice(const std::string& source,
		const std::string& target,
		const std::string& message)
{
	return ":" + source + " NOTICE " + target + " :" + message + "\r\n";
}








// WHO/WHOIS Command Replies

inline std::string rplWhoReply(const std::string &serverName,
		const std::string &requestorNick,
		const std::string &mask,
		const std::string &user,
		const std::string &host,
		const std::string &server,
		const std::string &nick,
		char status,
		const std::string &realName)
{
	return ":" + serverName + " 352 " + requestorNick + " " + mask + " " +
		user + " " + host + " " + server + " " + nick + " " +
		status + " :0 " + realName + "\r\n";
}

// 315 RPL_ENDOFWHO
inline std::string rplEndOfWho(const std::string &serverName,
		const std::string &requestorNick,
		const std::string &mask)
{
	return ":" + serverName + " 315 " + requestorNick + " " + mask +
		" :End of /WHO list\r\n";
}

// WHOIS reply codes
inline std::string rplWhoisUser(const std::string &serverName,
		const std::string &requestorNick,
		const std::string &targetNick,
		const std::string &user,
		const std::string &host,
		const std::string &realName)
{
	return ":" + serverName + " 311 " + requestorNick + " " +
		targetNick + " " + user + " " + host + " * :" + realName + "\r\n";
}


// If the user is an IRC operator:
inline std::string rplWhoisOperator(const std::string &serverName,
		const std::string &requestorNick,
		const std::string &targetNick)
{
	return ":" + serverName + " 313 " + requestorNick + " " + targetNick +
		" :is an IRC operator\r\n";
}

// 318 RPL_ENDOFWHOIS
inline std::string rplEndOfWhois(const std::string &serverName,
		const std::string &requestorNick,
		const std::string &targetNick)
{
	return ":" + serverName + " 318 " + requestorNick + " " + targetNick +
		" :End of /WHOIS list\r\n";
}
