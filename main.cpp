/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lstorey <lstorey@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:09:51 by cesasanc          #+#    #+#             */
/*   Updated: 2025/02/28 14:31:06 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <csignal>


/* Function to check if the port number is valid. */
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

/* Signal handler to stop the server when signal SIGINT is received. */
void	signalHandler(int signal)
{
	if (serverInstance)
	{
		std::cout << "\nSignal " << signal << " received. Stopping server..." << std::endl;
		serverInstance->cleanExit();
	}
	exit(EXIT_SUCCESS);
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
		serverInstance = &server;
		signal(SIGINT, signalHandler);
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
