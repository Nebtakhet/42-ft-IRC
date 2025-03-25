/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:24:45 by cesasanc          #+#    #+#             */
/*   Updated: 2025/03/06 11:19:23 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <sys/socket.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>
# include <vector>
# include <unordered_map>
# include <queue>
# include "Client.hpp"

class Server
{
public:
    Server(int port, const std::string &password);
    ~Server();
    void setupSocket();
    void handleConnections();
    void handleClient(int clientFd);
    void removeClient(int clientFd);
    void closeServer();
    void sendMessage();
    void messageBuffer(int clientFd, const std::string &message);
    void run();
    void cleanExit();
    void handleIncomingMessage(const std::string &message, int clientFd);
    void handleNickCommand(int clientFd, const std::string &nickname);
    void handleCapLs(int clientFd);
    void handleCapReq(int clientFd, const std::vector<std::string> &capabilities);
    void handleCapEnd(int clientFd);
    void sendToClient(int clientFd, const std::string &message);
    void handleJoinCommand(int clientFd, const std::string &channel); // Add this line

private:
    int port;
    std::string password;
    int serverSocket;
    bool running;
    struct sockaddr_in serverAddress;
    std::vector<struct pollfd> pollfds;
    std::unordered_map<int, std::string> clientBuffer;
    std::vector<Client> clients;
    std::unordered_map<std::string, std::vector<int>> channels; // Add this line

    const std::string DEFAULT_CHANNEL = "#default"; // Add this line
};

extern Server *serverInstance;

#endif