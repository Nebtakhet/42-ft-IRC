/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbejar-s <dbejar-s@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 11:23:17 by dbejar-s          #+#    #+#             */
/*   Updated: 2025/04/25 11:27:39 by dbejar-s         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "Server.hpp"
# include "Client.hpp"
# include "Parsing.hpp"

void nick(Server *server, int clientFd, const cmd_syntax &parsed);
void cap(Server *server, int clientFd, const cmd_syntax &parsed);
void join(Server *server, int clientFd, const cmd_syntax &parsed);
void user(Server *server, int clientFd, const cmd_syntax &parsed);
void pass(Server *server, int clientFd, const cmd_syntax &parsed);
void ping(Server *server, int clientFd, const cmd_syntax &parsed);
void part(Server *server, int clientFd, const cmd_syntax &parsed);
void privmsg(Server *server, int clientFd, const cmd_syntax &parsed);
void help(Server *server, int clientFd, const cmd_syntax &parsed);
void who(Server *server, int clientFd, const cmd_syntax &parsed);
void quit(Server *server, int clientFd, const cmd_syntax &parsed);
void kick(Server *server, int clientFd, const cmd_syntax &parsed);
void invite(Server *server, int clientFd, const cmd_syntax &parsed);
void topic(Server *server, int clientFd, const cmd_syntax &parsed);
void mode(Server *server, int clientFd, const cmd_syntax &parsed);

#endif