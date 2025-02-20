# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mpellegr <mpellegr@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/14 13:28:34 by pleander          #+#    #+#              #
#    Updated: 2025/02/18 06:37:14 by mpellegr         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME := ircserv
CXX := c++
CXXFLAGS := -Wall -Wextra -g -std=c++14
SOURCES := src/main.cpp src/Server.cpp src/User.cpp src/Logger.cpp src/Channel.cpp src/Message.cpp
INCLUDES := include
OBJECTS := $(SOURCES:.cpp=.o)

LOG_LEVEL := DEBUG #DEBUG/INFO/WARNING/ERROR

.PHONY: all
all: $(NAME)

$(NAME): $(OBJECTS)
		$(CXX) $(OBJECTS) -o $(NAME)

%.o: %.cpp
		$(CXX) -DLOG_LEVEL=$(LOG_LEVEL) $(CXXFLAGS) -I $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
		rm -f $(OBJECTS)

.PHONY: fclean
fclean: clean
		rm -f $(NAME)

.PHONY: re
re: fclean all
