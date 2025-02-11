/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/02/11 15:33:19 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port, const std::string &password) 
	: port(port), password(password), serverSocket(-1)
{
	setupSocket();
}

Server::~Server()
{
	closeServer();
}

void	Server::setupSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "Failed to create socket" << std::endl;
		exit(EXIT_FAILURE);
	}
	
	int flags = fcntl(serverSocket, F_GETFL, 0);
	if (flags == -1)
	{
		std::cerr << "Failed to get socket flags" << std::endl;
		exit(EXIT_FAILURE);
	}
	fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK);
	
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);
	
	if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
	{
		std::cerr << "Failed to bind socket" << std::endl;
		exit(EXIT_FAILURE);
	}

	if (listen(serverSocket, 10) == -1)
	{
		std::cerr << "Failed to listen on socket" << std::endl;
		exit(EXIT_FAILURE);
	}
}

void	Server::closeServer()
{
	close(serverSocket);
}

void	Server::run()
{
	std::cout << "Server running on port " << port << " with password " << password << std::endl;
}
