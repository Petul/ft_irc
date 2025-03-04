# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/14 13:28:34 by pleander          #+#    #+#              #
#    Updated: 2025/02/28 04:38:23 by jmakkone         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := ircserv
BOT_NAME := bot
CXX := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++14
SOURCES := src/main.cpp src/Server.cpp src/User.cpp src/Logger.cpp src/Channel.cpp src/Message.cpp
BOT_SOURCES := src/bot/main.cpp src/bot/Bot.cpp src/bot/GuardedChannel.cpp
INCLUDES := include
OBJECTS := $(SOURCES:.cpp=.o)
BOT_OBJECTS := $(BOT_SOURCES:.cpp=.o)

LOG_LEVEL := INFO #DEBUG/INFO/WARNING/ERROR

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
		$(CXX) $(OBJECTS) -o $(NAME)

.PHONY: bot
bot: $(BOT_OBJECTS)
	$(CXX) $(BOT_OBJECTS) -o $(BOT_NAME)

%.o: %.cpp
		$(CXX) -DLOG_LEVEL=$(LOG_LEVEL) $(CXXFLAGS) -I $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
		rm -f $(OBJECTS) $(BOT_OBJECTS)

.PHONY: fclean
fclean: clean
		rm -f $(NAME) $(BOT_NAME)

.PHONY: re
re: fclean all
