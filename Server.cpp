/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/03/19 22:02:53 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Parsing.hpp"
#include "Commands.hpp"

Server *serverInstance = nullptr;

/* Constructor calling setupSocket */
Server::Server(int port, const std::string &password) 
	: port(port), password(password), serverSocket(-1), running(false)
{
	std::cout << "Initializing server on port " << port << " with password " << password << std::endl;
	setupSocket();
}

/* Destructor calling closeServer */
Server::~Server()
{
	closeServer();
}

void Server::handleIncomingMessage(const std::string &message, int clientFd) {
    try {
        std::istringstream stream(message);
        std::string line;
        while (std::getline(stream, line)) {
            if (line.back() == '\r') {
                line.pop_back();
            }
            cmd_syntax parsed = parseIrcMessage(line);

            if (parsed.name == "NICK") {
                handleNickCommand(clientFd, parsed.params[0]);
            } else if (parsed.name == "CAP") {
                cap(this, clientFd, parsed);
            } else if (parsed.name == "JOIN") {
                join(this, clientFd, parsed);
            } else if (parsed.name == "USER") {
                user(this, clientFd, parsed);
            } else if (parsed.name == "PASS") {
                pass(this, clientFd, parsed);
            } else if (parsed.name == "PING") {
                ping(this, clientFd, parsed);
            } else {
                std::cerr << "ERROR: Unknown command.\n";
            }
        }
    } catch (const std::exception &e) {
        std::cerr << "Message parsing error: " << e.what() << std::endl;
    }
}

void Server::handleNickCommand(int clientFd, const std::string &nickname) {
    // Find the client by clientFd
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end()) {
        it->setNickname(nickname);
        std::cout << "Client " << clientFd << " set nickname to " << nickname << std::endl;
    } else {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
    }
}

void Server::sendToClient(int clientFd, const std::string &message) {
    send(clientFd, message.c_str(), message.size(), 0);

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm *now_tm = std::localtime(&now_time);

    std::ostringstream time_stream;
    time_stream << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");

    // DEBUGGING
    std::cout << time_stream.str() << " ******************** Sent to client " << clientFd << " >>> " << message << std::endl;
}

void Server::handleCapLs(int clientFd) {
    std::string capList = "multi-prefix sasl";
    std::string response = "CAP * LS :" + capList + "\r\n";
    sendToClient(clientFd, response); 
}

void Server::handleCapReq(int clientFd, const std::vector<std::string> &capabilities) {
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end()) {
        for (const auto &cap : capabilities) {
            it->addCapability(cap);
        }
        std::string response = "CAP * ACK :" + capabilities[0] + "\r\n";
        sendToClient(clientFd, response); 
    }
}

void Server::handleCapEnd(int clientFd) {
    std::string response = "CAP * END\r\n";
    sendToClient(clientFd, response); 
}

void Server::handleJoinCommand(int clientFd, const std::string &channel)
{
    std::string joinChannel = channel.empty() ? DEFAULT_CHANNEL : channel;

    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end())
    {
        channels[joinChannel].push_back(clientFd);
        std::string response = ":" + it->getNickname() + " JOIN " + joinChannel + "\r\n";
        sendToClient(clientFd, response);
        std::cout << "Client " << clientFd << " joined channel " << joinChannel << std::endl;
    }
    else
    {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
    }
}

void Server::handleUserCommand(int clientFd, const std::string &username, const std::string &hostname, const std::string &servername, const std::string &realname)
{
    (void)hostname;
    (void)servername;

    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end())
    {
        it->setUsername(username);
        it->setRealname(realname);
        std::cout << "Client " << clientFd << " set username to " << username << " and realname to " << realname << std::endl;
    }
    else
    {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
    }
}

void Server::handlePassCommand(int clientFd, const std::string &password)
{
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end())
    {
        if (password == this->password)
        {
            it->setAuthenticated(true);
            std::cout << "Client " << clientFd << " authenticated successfully" << std::endl;
        }
        else
        {
            std::cerr << "Client " << clientFd << " provided incorrect password" << std::endl;
            removeClient(clientFd);
        }
    }
    else
    {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
    }
}
