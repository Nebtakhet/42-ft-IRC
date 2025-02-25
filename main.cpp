/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lstorey <lstorey@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:09:51 by cesasanc          #+#    #+#             */
/*   Updated: 2025/02/25 14:57:15 by lstorey          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Commands.hpp"

bool validPort(const char *str, int &port)
{
	char *end;
	errno = 0;
	long	val = std::strtol(str, &end, 10);

	if (errno == ERANGE || *end != '\0' || val < 1 || val > 65535)
		return (false);
	port = static_cast<int>(val);
	return (true);
}

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv [port] [password]" << std::endl;
		return (EXIT_FAILURE);
	}
	
	int port;
	if (!validPort(argv[1], port))
	{
		std::cerr << "Please enter a valid port number." << std::endl;
		return (EXIT_FAILURE);
	}
	
	std::string password = argv[2];

	try
	{
		Server server(port, password);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	/*  ------ MESSAGE PARSING ------
	
	std::string irc_message = ":Leo PRIVMSG #channel :Hello, world!\r\n";
    cmd_syntax parsed = parse_irc_message(irc_message);
	

	*/

	return (EXIT_SUCCESS);
}
