/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/02/12 21:47:59 by cesasanc         ###   ########.fr       */
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

	pollfds.push_back({serverSocket, POLLIN, 0});
}

void	Server::handleConnections()
{
	struct sockaddr_in clientAddress;
	socklen_t	clientLen = sizeof(clientAddress);
	int clientFd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);
	
	if (clientFd >= 0)
	{
		fcntl(clientFd, F_SETFL, O_NONBLOCK);
		pollfds.push_back({clientFd, POLLIN, 0});
		std::cout << "New client connected: " << clientFd << std::endl;
	}
	
}

void	Server::handleClient(int clientFd)
{
	char	buffer[512];
	int		bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		std::cout << "Received message from " << clientFd << ": " << buffer << std::endl;
		send(clientFd, buffer, bytesRead, 0);
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client " << clientFd << " disconnected" << std::endl;
		close(clientFd);
		removeClient(clientFd);
	}
	else
	{
		std::cerr << "Failed to receive message from " << clientFd << std::endl;
		close(clientFd);
		removeClient(clientFd);
	}
}	

void	Server::removeClient(int clientFd)
{
	close(clientFd);
	for (size_t i = 0; i < pollfds.size(); i++)
	{
		if (pollfds[i].fd == clientFd)
		{
			pollfds.erase(pollfds.begin() + i);
			break;
		}
	}
}

void	Server::closeServer()
{
	for (size_t i = 0; i < pollfds.size(); i++)
	{
		if (pollfds[i].fd != serverSocket)
			close(pollfds[i].fd);
	}
	pollfds.clear();
	close(serverSocket);
}

void	Server::run()
{
	std::cout << "Server running on port " << port << " with password " << password << std::endl;
	while (true)
	{
		int ret = poll(pollfds.data(), pollfds.size(), -1);
		if (ret == -1)
		{
			std::cerr << "Poll failed" << std::endl;
			break;
		}
		
		for (size_t i = 0; i < pollfds.size(); i++)
		{
			if (pollfds[i].revents & POLLIN)
			{
				if (pollfds[i].fd == serverSocket)
					handleConnections();
				else
					handleClient(pollfds[i].fd);
			}
		}		
	}
}
