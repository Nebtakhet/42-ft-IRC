#ifndef COMMANDS_HPP
# define COMMANDS_HPP

#include "Server.hpp"

class Server;

struct cmd_syntax
{
	std::string	prefix;
	std::string	name;
	std::string	message;
};

//Commands

void	invite(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	join(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	kick(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	kill(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	list(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	modeFunction(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	motd(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	names(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	nick(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	notice(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	oper(Server *server, int const clientFd, cmd_syntax actual_cmd);
int		pass(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	part(Server *server, int const clientFd, cmd_syntax actual_cmd);
int		ping(Server *server, int const clientFd, cmd_syntax &cmd);
void	privmsg(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	quit(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	topic(Server *server, int const clientFd, cmd_syntax actual_cmd);
void	user(Server *server, int const clientFd, cmd_syntax actual_cmd);

#endif