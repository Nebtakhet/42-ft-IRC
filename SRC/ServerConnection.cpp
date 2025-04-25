/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConnection.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbejar-s <dbejar-s@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 11:24:09 by dbejar-s          #+#    #+#             */
/*   Updated: 2025/04/25 11:35:58 by dbejar-s         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Parsing.hpp"
#include "Commands.hpp"

void Server::setupSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
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
        std::cerr << "Failed to set socket options" << std::endl;
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
    std::cout << "Socket setup complete. Listening on port " << port << std::endl;
}

void Server::handleConnections()
{
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientFd;

    while ((clientFd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen)) >= 0)
    {
        if (clients.size() >= MAX_CLIENTS)
        {
            std::cerr << "Maximum number of clients reached. Rejecting connection from client " << clientFd << std::endl;
            std::string response = "ERROR :Server full. Maximum number of clients reached.\r\n";
            send(clientFd, response.c_str(), response.size(), 0);
            close(clientFd);
            continue;
        }

        if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
        {
            std::cerr << "Failed to set client socket to non-blocking" << std::endl;
            close(clientFd);
            continue;
        }

        pollfds.push_back({clientFd, POLLIN, 0});
        clients.emplace_back(clientFd); 
        std::cout << "New client connected: " << clientFd << std::endl;
    }

    if (errno == EWOULDBLOCK || errno == EAGAIN)
        return;
    std::cerr << "Failed to accept client connection: " << strerror(errno) << std::endl;
}

void Server::handleClient(int clientFd)
{
    char buffer[512];
    int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        clientBuffer[clientFd] += std::string(buffer, bytesRead);

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm *now_tm = std::localtime(&now_time);

        std::ostringstream time_stream;
        time_stream << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");

        std::cout << time_stream.str() << " >>>>>>>>>>>>>>>>>>>> Received from client " << clientFd << " >>> " << buffer << std::endl;

        size_t pos;
        while ((pos = clientBuffer[clientFd].find('\n')) != std::string::npos)
        {
            std::string command = clientBuffer[clientFd].substr(0, pos);
            clientBuffer[clientFd].erase(0, pos + 1);

            if (!command.empty() && command.back() == '\r') {
                command.pop_back();
            }

            std::cout << "Client " << clientFd << ": " << command << std::endl;
            handleIncomingMessage(command, clientFd);
        }
    }
    else if (bytesRead == 0)
    {
        std::cout << "Client " << clientFd << " disconnected" << std::endl;
        removeClient(clientFd);
    }
    else if (bytesRead == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return;
        std::cerr << "Failed to receive message from " << clientFd << ": " << strerror(errno) << std::endl;
        removeClient(clientFd);
    }
}

void Server::removeClient(int clientFd)
{
    close(clientFd);

    for (size_t i = pollfds.size(); i-- > 0;)
    {
        if (pollfds[i].fd == clientFd)
        {
            pollfds.erase(pollfds.begin() + i);
            break;
        }
    }

    clientBuffer.erase(clientFd);

    clients.erase(std::remove_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    }), clients.end());

    for (auto it = channels.begin(); it != channels.end();)
    {
        Channel &channel = it->second;
        channel.removeMember(clientFd);

        if (channel.getMembers().empty())
        {
            std::cout << "Channel " << channel.getName() << " is now empty and has been removed" << std::endl;
            it = channels.erase(it);
        }
        else
            ++it;
    }

    std::cout << "Client " << clientFd << " removed" << std::endl;
}

void Server::closeServer()
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

void Server::sendMessage()
{
    for (size_t i = pollfds.size(); i-- > 0;)
    {
        if (pollfds[i].revents & POLLOUT)
        {
            int clientFd = pollfds[i].fd;
            if (clientBuffer.find(clientFd) != clientBuffer.end() && !clientBuffer[clientFd].empty())
            {
                std::string &message = clientBuffer[clientFd];

                ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0);
                if (bytesSent > 0)
                {
                    message.erase(0, bytesSent);
                    if (message.empty())
                    {
                        clientBuffer[clientFd].clear();
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

void Server::messageBuffer(int clientFd, const std::string &message)
{
    clientBuffer[clientFd] += message;

    for (size_t i = 0; i < pollfds.size(); i++)
    {
        if (pollfds[i].fd == clientFd)
        {
            pollfds[i].events |= POLLOUT;
            break;
        }
    }
}

void Server::run()
{
    std::cout << "Server running on port " << port << " with password " << password << std::endl;
    running = true;
    while (running)
    {
        int ret = poll(pollfds.data(), pollfds.size(), -1);
        if (ret == -1)
        {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }

        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            if (pollfds[i].revents & POLLIN)
            {
                if (pollfds[i].fd == serverSocket)
                {
                    handleConnections();
                }
                else
                {
                    handleClient(pollfds[i].fd);
                }
            }
        }
    }
}

void Server::cleanExit()
{
	for (const auto &pollfd : pollfds)
	{
		if (pollfd.fd != serverSocket)
			removeClient(pollfd.fd);
	}
		
    closeServer();
    exit(EXIT_SUCCESS);
}