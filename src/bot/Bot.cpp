/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jmakkone <jmakkone@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 03:57:30 by jmakkone          #+#    #+#             */
/*   Updated: 2025/02/28 04:36:26 by jmakkone         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <iostream>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <atomic>

std::atomic<bool> g_shouldQuit{false};

static void signalHandler(int)
{
    g_shouldQuit = true;
}

Bot::Bot(const std::string& hostname, int port, const std::string& password)
    : hostname_(hostname), port_(port), password_(password), sockfd_(-1)
{
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    sigaction(SIGQUIT, &sa, nullptr);
    sigaction(SIGPIPE, &sa, nullptr);
}

Bot::~Bot()
{
    closeConnection();
}

void Bot::run()
{
    struct hostent *hostEntry = gethostbyname(hostname_.c_str());
    if (!hostEntry)
    {
        throw std::runtime_error("Invalid hostname: " + hostname_);
    }
    sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd_ == -1)
    {
        throw std::runtime_error("Socket creation failed");
    }
    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(port_);
    in_addr.sin_addr.s_addr = *hostEntry->h_addr;
    if (connect(sockfd_, (sockaddr*)(&in_addr), sizeof(in_addr)) == -1)
    {
        close(sockfd_);
        throw std::runtime_error("Failed to connect to " + hostname_);
    }

    std::string auth = "PASS " + password_ + "\r\n" +
                       "NICK bot\r\n" +
                       "USER bot 0 * thisisabot\r\n";
    if (send(sockfd_, auth.c_str(), auth.size(), 0) == -1)
    {
        throw std::runtime_error("Failed to send auth to the server");
    }

    char inputBuf[BUFFER_SIZE];
    int rbytes = 0;
    while (!g_shouldQuit)
    {
        std::memset(inputBuf, 0, BUFFER_SIZE);
        rbytes = recv(sockfd_, inputBuf, BUFFER_SIZE, 0);
        if (rbytes == -1)
        {
            std::cerr << "Data feed from socket interrupted\n";
            break;
        }
        else if (rbytes > 0)
        {
            inputBuf[rbytes] = '\0';
            std::string inputMsg(inputBuf);
            std::cout << "Received: " << inputMsg << "\n";
            try 
            {
                handleInput(inputMsg);
            }
            catch (std::exception& e)
            {
                std::cerr << "Input handling failed: " << e.what() << "\n";
            }
        }
    }

    sendCommand("QUIT :BOT quits, bye\r\n");
    closeConnection();
}

void Bot::sendCommand(const std::string& message)
{
    if (!message.empty())
    {
        if (send(sockfd_, message.c_str(), message.size(), 0) == -1)
        {
            std::cerr << "Sending failed\n";
        }
        std::cout << "Sent: " << message;
    }
}

std::string Bot::parseTarget(const std::string& message,
                             const std::string& tStart,
                             const std::string& tEnd)
{
    size_t start = message.find(tStart);
    if (start != std::string::npos)
    {
        size_t end = message.find_first_of(tEnd, start);
        if (end != std::string::npos)
        {
            return message.substr(start, end - start);
        }
        else
        {
            return message.substr(start);
        }
    }
    return "";
}

void Bot::handleInput(const std::string& inputMsg)
{
    if (inputMsg.find("PING") != std::string::npos)
    {
        std::string target = parseTarget(inputMsg, "PING ", " \r\n");
        sendCommand("PONG " + target + "\r\n");
    }
    else if (inputMsg.find("INVITE") != std::string::npos)
    {
        std::string channel = parseTarget(inputMsg, "#", " \r\n");
        sendCommand("JOIN " + channel + "\r\n");
        sendCommand("PRIVMSG " + channel + " :Hello! I'm a bot.\r\n");
    }
    else if (inputMsg.find("!BOT LEAVE") != std::string::npos)
    {
        std::string channel = parseTarget(inputMsg, "#", " \r\n");

        bool isGuarded = false;
        for (auto &gc : guardedChannels_)
        {
            if (gc.getChannel() == channel)
            {
                isGuarded = true;
                break;
            }
        }

        if (!isGuarded)
        {
            sendCommand("PART " + channel + " :Okay, bye!\r\n");
        }
        else
        {
            std::string owner = parseTarget(inputMsg, ":", " ");
            if (!owner.empty() && owner.front() == ':')
                owner.erase(0, 1);
            std::string password = parseTarget(inputMsg, "!pw ", "\r\n");

            bool found = false;
            for (auto &gc : guardedChannels_)
            {
                if (gc.getChannel() == channel &&
                    gc.getOwner() == owner &&
                    gc.getPassword() == password)
                {
                    found = true;
                    break;
                }
            }
            if (found)
            {
                sendCommand("PART " + channel + " :Okay, bye!\r\n");
            }
            else
            {
                sendCommand("PRIVMSG " + owner + " :I'm not your bot, go away\r\n");
            }
        }
    }
    else if (inputMsg.find("!BOT GUARD") != std::string::npos)
    {
        std::string owner = parseTarget(inputMsg, ":", " ");
        if (!owner.empty() && owner.front() == ':')
            owner.erase(0, 1);
        std::string channel = parseTarget(inputMsg, "#", " ");
        std::string password = parseTarget(inputMsg, "!pw ", "\r\n");
        if (!owner.empty() && !channel.empty() && !password.empty())
        {
            guardedChannels_.push_back(GuardedChannel(owner, channel, password));
            std::cout << "Guarded channel saved: " << channel << " for owner " << owner << "\n";
        }
    }
    else if (inputMsg.find("!BOT OP") != std::string::npos)
    {
        std::string owner = parseTarget(inputMsg, ":", " ");
        if (!owner.empty() && owner.front() == ':')
            owner.erase(0, 1);
        std::string channel = parseTarget(inputMsg, "#", " ");
        std::string password = parseTarget(inputMsg, "!pw ", "\r\n");
        bool found = false;
        for (auto &gc : guardedChannels_)
        {
            if (gc.getOwner() == owner && gc.getChannel() == channel &&
                gc.getPassword() == password)
            {
                found = true;
                break;
            }
        }
        if (found)
        {
            sendCommand("MODE " + channel + " +o " + owner + "\r\n");
            sendCommand("PRIVMSG " + channel + " :Operator privilege granted to " + owner + "\r\n");
        }
        else
        {
            sendCommand("PRIVMSG " + owner + " :I'm not your bot, go away\r\n");
        }
    }
}

void Bot::closeConnection()
{
    if (sockfd_ != -1)
    {
        close(sockfd_);
        sockfd_ = -1;
    }
}
