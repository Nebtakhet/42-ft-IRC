/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:09:51 by cesasanc          #+#    #+#             */
/*   Updated: 2025/03/19 22:00:14 by cesasanc         ###   ########.fr       */
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
	exit(signal);
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

	//if(password.length < 5) // maybe a password function to make sure its sensible, nothing in the protocol tho
	//{
	//	std::cerr << "Please enter a proper password you lazy fuck!!!!." << std::endl;
	//	return (EXIT_FAILURE);
	//} 
	
	try
	{
		Server server(port, password);
		serverInstance = &server;
		
		signal(SIGINT, signalHandler);
		signal(SIGTERM, signalHandler);
		signal(SIGQUIT, signalHandler);
		signal(SIGHUP, signalHandler);
		
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}
