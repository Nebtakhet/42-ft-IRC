/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/03/06 11:01:03 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Parsing.hpp"

Server *serverInstance = nullptr;

/* Constructor calling setupSocket */
Server::Server(int port, const std::string &password) 
	: port(port), password(password), serverSocket(-1), running(true)
{
	setupSocket();
}

/* Destructor calling closeServer */
Server::~Server()
{
	closeServer();
}

/* Function to setup the socket. It creates a socket, sets it to non-blocking, 
binds it to the server address and listens on it. It also adds the server socket
to the pollfds vector. */
void	Server::setupSocket()
{
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		std::cerr << "Failed to create socket: " <<  strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	
	if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Failed to set server socket to non-blocking" << std::endl;
		exit(EXIT_FAILURE);
	}

	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
		exit(EXIT_FAILURE);
	}
	
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

/* Function to handle incoming connections. It accepts the connection, sets the client
socket to non-blocking and adds it to the pollfds vector. */
void	Server::handleConnections()
{
	struct sockaddr_in	clientAddress;
	socklen_t			clientLen = sizeof(clientAddress);
	int 				clientFd;
	
	while ((clientFd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen)) >= 0)
	{
		if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
		{
			std::cerr << "Failed to set client socket to non-blocking" << std::endl;
			close(clientFd);
			continue;
		}
		pollfds.push_back({clientFd, POLLIN, 0});
		std::cout << "New client connected: " << clientFd << std::endl;
	}
	
	if (errno == EWOULDBLOCK && errno == EAGAIN)
		return;
	std::cerr << "Failed to accept client connection: " << strerror(errno) << std::endl;
}

/* Function to handle incoming messages from clients. It reads the message, prints it
to the console and adds it to the clientBuffer. If the client disconnects, it removes
the client from the pollfds vector and closes the connection. */
void	Server::handleClient(int clientFd)
{
	char	buffer[512];
	int		bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
	
	if (bytesRead > 0)
	{
		buffer[bytesRead] = '\0';
		std::cout << "Received message from " << clientFd << ": " << buffer << std::endl;
		messageBuffer(clientFd, std::string(buffer, bytesRead));
		// inputted parsing function  here //
		handleIncomingMessage(std::string(buffer, bytesRead), clientFd);
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client " << clientFd << " disconnected" << std::endl;
		removeClient(clientFd);
	}
	else if (bytesRead == -1)
	{
		if (errno == EWOULDBLOCK && errno == EAGAIN)
			return;
		std::cerr << "Failed to receive message from " << clientFd << ": " << strerror(errno) << std::endl;
		removeClient(clientFd);
	}
}	

/* Function to remove a client from the pollfds vector and close the connection. */
void	Server::removeClient(int clientFd)
{
    close(clientFd);
    for (size_t i = pollfds.size(); i-- > 0; )
    {
        if (pollfds[i].fd == clientFd)
        {
            pollfds.erase(pollfds.begin() + i);
            break;
        }
    }
    clientBuffer.erase(clientFd);
}

/* Function to close the server. It closes all client connections and the server socket. */
void	Server::closeServer()
{
	for (size_t i = 0; i < pollfds.size(); i++)
	{
		if (pollfds[i].fd != serverSocket)
			close(pollfds[i].fd);
	}
	close(serverSocket);
	pollfds.clear();
	clientBuffer.clear();
	running = false;
}

/* Function to send messages to clients. It iterates through the pollfds vector and
sends messages to clients that have messages in the clientBuffer. */
void	Server::sendMessage()
{
    for (size_t i = pollfds.size(); i-- > 0; )
    {
        if (pollfds[i].revents & POLLOUT)
        {
            int clientFd = pollfds[i].fd;
            if (clientBuffer.find(clientFd) != clientBuffer.end() && !clientBuffer[clientFd].empty())
            {
                std::string &message = clientBuffer[clientFd].front();
                
                ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0);
                if (bytesSent > 0)
                {
                    message.erase(0, bytesSent);
                    if (message.empty())
                    {
                        clientBuffer[clientFd].pop();
                        if (clientBuffer[clientFd].empty())
                            pollfds[i].events &= ~POLLOUT;
                    }
                }
                else if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    std::cerr << "Failed to send message to " << clientFd << std::endl;
                    removeClient(clientFd);
                }
            }
        }
    }
}

/* Function to add a message to the clientBuffer and set the POLLOUT event. */
void	Server::messageBuffer(int clientFd, const std::string &message)
{
	clientBuffer[clientFd].push(message);
	
	for (size_t i = 0; i < pollfds.size(); i++)
	{
		if (pollfds[i].fd == clientFd)
		{
			pollfds[i].events |= POLLOUT;
			break;
		}
	}
}

/* Function to run the server. It polls the pollfds vector and calls handleConnections
and handleClient when there is activity. It also calls sendMessage to send messages to
clients. */
void	Server::run()
{
    std::cout << "Server running on port " << port << " with password " << password << std::endl;
       while (running)
       {
        int ret = poll(pollfds.data(), pollfds.size(), 1000);
        if (ret == -1)
        {
            std::cerr << "Poll failed: " << strerror(errno) << std::endl;
            break;
        }
        
        for (size_t i = pollfds.size(); i-- > 0; )
        {
            if (pollfds[i].revents & POLLIN)
            {
                if (pollfds[i].fd == serverSocket)
                    handleConnections();
                else
                    handleClient(pollfds[i].fd);
            }
        }
        sendMessage();	
    }
}

/* Function to handle a clean exit. It closes the server and exits with EXIT_SUCCESS. */
void	Server::cleanExit()
{
	closeServer();
	exit(EXIT_SUCCESS);
}

 /* Function to catch any problems(general) and parse into cmd_syntax */
 void Server::handleIncomingMessage(const std::string &message, int clientFd)
 {
	 try
	 {
		 cmd_syntax parsed = parseIrcMessage(message);
 
		 // Redirect to the appropriate command function
		 if (parsed.name == "JOIN")
		 {
			 join(this, clientFd, parsed);
		 }
		 else if (parsed.name == "INVITE")
		 {
			 invite(this, clientFd, parsed);
		 }
		 else if (parsed.name == "KICK")
		 {
			 kick(this, clientFd, parsed);
		 }
		 else if (parsed.name == "KILL")
		 {
			 kill(this, clientFd, parsed);
		 }
		 else if (parsed.name == "LIST")
		 {
			 list(this, clientFd, parsed);
		 }
		 else if (parsed.name == "MODE")
		 {
			 modeFunction(this, clientFd, parsed);
		 }
		 else if (parsed.name == "MOTD")
		 {
			 motd(this, clientFd, parsed);
		 }
		 else if (parsed.name == "NAMES")
		 {
			 names(this, clientFd, parsed);
		 }
		 else if (parsed.name == "NICK")
		 {
			 nick(this, clientFd, parsed);
		 }
		 else if (parsed.name == "NOTICE")
		 {
			 notice(this, clientFd, parsed);
		 }
		 else if (parsed.name == "OPER")
		 {
			 oper(this, clientFd, parsed);
		 }
		 else if (parsed.name == "PASS")
		 {
			 pass(this, clientFd, parsed);
		 }
		 else if (parsed.name == "PART")
		 {
			 part(this, clientFd, parsed);
		 }
		 else if (parsed.name == "PING")
		 {
			 ping(this, clientFd, parsed);
		 }
		 else if (parsed.name == "PRIVMSG")
		 {
			 privmsg(this, clientFd, parsed);
		 }
		 else if (parsed.name == "QUIT")
		 {
			 quit(this, clientFd, parsed);
		 }
		 else if (parsed.name == "TOPIC")
		 {
			 topic(this, clientFd, parsed);
		 }
		 else if (parsed.name == "USER")
		 {
			 user(this, clientFd, parsed);
		 }
		 else
		 {
			 std::cerr << "ERROR: Unknown command.\n";
		 }
	 }
	 catch (const std::exception &e)
	 {
		 std::cerr << "Message parsing error: " << e.what() << std::endl;
	 }
 }
 
