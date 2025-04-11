# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/02/06 12:52:59 by cesasanc          #+#    #+#              #
#    Updated: 2025/04/08 20:05:17 by cesasanc         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRCDIR = SRC
OBJDIR = OBJECTS

SRCS =	main.cpp \
        Server.cpp \
		ServerConnection.cpp \
        Client.cpp \
        Parsing.cpp \
        Commands.cpp \
		ChannelOperators.cpp \

SRCS := $(addprefix $(SRCDIR)/, $(SRCS))
OBJS = $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

CFLAGS	=	-Wall -Wextra -Werror

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	@c++ $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJS)
	@c++ $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	@rm -rf $(OBJDIR)

fclean: clean
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re