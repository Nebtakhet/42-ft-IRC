#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Server.hpp"
#include "Client.hpp"
#include "Parsing.hpp"

void nick(Server *server, int clientFd, const cmd_syntax &parsed);
void cap(Server *server, int clientFd, const cmd_syntax &parsed);
void join(Server *server, int clientFd, const cmd_syntax &parsed);

#endif