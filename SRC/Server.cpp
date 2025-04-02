/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/04/02 11:56:16 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Parsing.hpp"
#include "Commands.hpp"
#include "Client.hpp"

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
            } else if (parsed.name == "PART") {
                part(this, clientFd, parsed);
            } else if (parsed.name == "USER") {
                user(this, clientFd, parsed);
            } else if (parsed.name == "PASS") {
                pass(this, clientFd, parsed);
            } else if (parsed.name == "PING") {
                ping(this, clientFd, parsed);
            } else if (parsed.name == "PRIVMSG") {
                privmsg(this, clientFd, parsed);
            } else if (parsed.name == "HELP") {
                help(this, clientFd, parsed);
            } else if (parsed.name == "WHO") {
                who(this, clientFd, parsed);
            } else if (parsed.name == "QUIT") {
                quit(this, clientFd, parsed);
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

void Server::handlePartCommand(int clientFd, const std::string &channel) {
    auto it = channels.find(channel);
    if (it == channels.end()) {
        std::cerr << "Channel " << channel << " does not exist" << std::endl;
        std::string response = "403 " + channel + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
        sendToClient(clientFd, response);
        return;
    }

    auto &clientsInChannel = it->second;
    auto clientIt = std::find(clientsInChannel.begin(), clientsInChannel.end(), clientFd);
    if (clientIt == clientsInChannel.end()) {
        std::cerr << "Client " << clientFd << " is not in channel " << channel << std::endl;
        std::string response = "442 " + channel + " :You're not on that channel\r\n"; // ERR_NOTONCHANNEL
        sendToClient(clientFd, response);
        return;
    }

    // Remove the client from the channel
    clientsInChannel.erase(clientIt);
    std::cout << "Client " << clientFd << " left channel " << channel << std::endl;

    // Notify the client and the channel
    std::string response = ":" + clients[clientFd].getNickname() + " PART " + channel + "\r\n";
    sendToClient(clientFd, response);

    // If the channel is empty, remove it
    if (clientsInChannel.empty()) {
        channels.erase(it);
        std::cout << "Channel " << channel << " is now empty and has been removed" << std::endl;
    }
}

void Server::handlePrivmsgCommand(int clientFd, const std::string &target, const std::string &message) {
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it == clients.end()) {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    std::string sender = it->getNickname();

    // Check if the target is a channel
    if (target[0] == '#') {
        auto channelIt = channels.find(target);
        if (channelIt == channels.end()) {
            std::cerr << "Channel " << target << " does not exist" << std::endl;
            std::string response = "403 " + target + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
            sendToClient(clientFd, response);
            return;
        }

        // Send the message to all clients in the channel except the sender
        for (int memberFd : channelIt->second) {
            if (memberFd != clientFd) {
                std::string response = ":" + sender + " PRIVMSG " + target + " :" + message + "\r\n";
                sendToClient(memberFd, response);
            }
        }
    } else {
        // Target is a user
        auto targetIt = std::find_if(clients.begin(), clients.end(), [&target](const Client &client) {
            return client.getNickname() == target;
        });

        if (targetIt == clients.end()) {
            std::cerr << "User " << target << " does not exist" << std::endl;
            std::string response = "401 " + target + " :No such nick/channel\r\n"; // ERR_NOSUCHNICK
            sendToClient(clientFd, response);
            return;
        }

        // Send the message to the target user
        int targetFd = targetIt->getClientFd();
        std::string response = ":" + sender + " PRIVMSG " + target + " :" + message + "\r\n";
        sendToClient(targetFd, response);
    }
}

void Server::handleHelpCommand(int clientFd) {
    std::string response =
        "Available commands:\r\n"
        "NICK <nickname> - Set your nickname\r\n"
        "USER <username> <hostname> <servername> :<realname> - Register your username\r\n"
        "JOIN <#channel> - Join a channel\r\n"
        "PART <#channel> - Leave a channel\r\n"
        "PRIVMSG <target> <message> - Send a private message to a user or channel\r\n"
        "PING <server> - Ping the server\r\n"
        "PASS <password> - Authenticate with the server\r\n"
        "WHO <target> - List information about users\r\n"
        "QUIT <message> - Disconnect from the server\r\n"
        "HELP - Show this help message\r\n"
        "\r\n"
        "Examples:\r\n"
        "  NICK JohnDoe\r\n"
        "  USER John localhost server :John Doe\r\n"
        "  JOIN #general\r\n"
        "  PART #general\r\n"
        "  PRIVMSG #general :Hello everyone!\r\n"
        "  PRIVMSG JohnDoe :Hello, John!\r\n"
        "  PING irc.example.com\r\n"
        "  QUIT :Goodbye!\r\n";

    sendToClient(clientFd, response);
}

void Server::handleWhoCommand(int clientFd, const std::string &target) {
    std::ostringstream response;

    if (target.empty()) {
        // WHO without a target: list all users on the server
        for (const Client &client : clients) {
            response << client.getNickname() << " "
                     << client.getUsername() << " "
                     << client.getRealname() << "\r\n";
        }
    } else if (target[0] == '#') {
        // WHO for a channel
        auto channelIt = channels.find(target);
        if (channelIt == channels.end()) {
            std::cerr << "Channel " << target << " does not exist" << std::endl;
            std::string errorResponse = "403 " + target + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
            sendToClient(clientFd, errorResponse);
            return;
        }

        for (int memberFd : channelIt->second) {
            auto it = std::find_if(clients.begin(), clients.end(), [memberFd](const Client &client) {
                return client.getClientFd() == memberFd;
            });

            if (it != clients.end()) {
                response << it->getNickname() << " "
                         << it->getUsername() << " "
                         << it->getRealname() << "\r\n";
            }
        }
    } else {
        // WHO for a specific user
        auto it = std::find_if(clients.begin(), clients.end(), [&target](const Client &client) {
            return client.getNickname() == target;
        });

        if (it == clients.end()) {
            std::cerr << "User " << target << " does not exist" << std::endl;
            std::string errorResponse = "401 " + target + " :No such nick/channel\r\n"; // ERR_NOSUCHNICK
            sendToClient(clientFd, errorResponse);
            return;
        }

        response << it->getNickname() << " "
                 << it->getUsername() << " "
                 << it->getRealname() << "\r\n";
    }

    sendToClient(clientFd, response.str());
}

void Server::handleQuitCommand(int clientFd, const std::string &quitMessage) {
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it == clients.end()) {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    std::string nickname = it->getNickname();

    // Notify all channels the client is part of
    for (auto &channel : channels) {
        auto &clientsInChannel = channel.second;
        if (std::find(clientsInChannel.begin(), clientsInChannel.end(), clientFd) != clientsInChannel.end()) {
            std::string response = ":" + nickname + " QUIT :" + quitMessage + "\r\n";
            for (int memberFd : clientsInChannel) {
                if (memberFd != clientFd) {
                    sendToClient(memberFd, response);
                }
            }
        }
    }

    // Remove the client from all channels
    for (auto &channel : channels) {
        auto &clientsInChannel = channel.second;
        clientsInChannel.erase(std::remove(clientsInChannel.begin(), clientsInChannel.end(), clientFd), clientsInChannel.end());
    }

    // Remove empty channels
    for (auto it = channels.begin(); it != channels.end();) {
        if (it->second.empty()) {
            it = channels.erase(it);
        } else {
            ++it;
        }
    }

    // Remove the client
    std::cout << "Client " << clientFd << " (" << nickname << ") disconnected with message: " << quitMessage << std::endl;
    removeClient(clientFd);
}

void Server::handleKickCommand(int clientFd, const std::string &channel, const std::string &target, const std::string &reason)
{
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it == clients.end())
    {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    auto channelIt = channels.find(channel);
    if (channelIt == channels.end())
    {
        std::cerr << "Channel " << channel << " does not exist" << std::endl;
        std::string response = "403 " + channel + " :No such channel\r\n";
        sendToClient(clientFd, response);
        return;
    }

    auto &clientsInChannel = channelIt->second;

    if (!it->hasCapability("Operator")){
        std::cerr << "Client " << clientFd << " does not have permission to kick users from channel " << channel << std::endl;
        std::string response = "482 " + channel + " :You're not channel operator\r\n";
        sendToClient(clientFd, response);
        return;
    }

    auto targetIt = std::find_if(clients.begin(), clients.end(), [&target](const Client &client) {
        return client.getNickname() == target;
    });

    if (targetIt == clients.end())
    {
        std::cerr << "User " << target << " does not exist" << std::endl;
        std::string response = "401 " + target + " :No such nick/channel\r\n"; 
        sendToClient(clientFd, response);
        return;
    }

    int targetFd = targetIt->getClientFd();

    auto clientInChannelIt = std::find(clientsInChannel.begin(), clientsInChannel.end(), targetFd);
    if (clientInChannelIt == clientsInChannel.end())
    {
        std::cerr << "User " << target << " is not in channel " << channel << std::endl;
        std::string response = "441 " + target + " " + channel + " :They aren't on that channel\r\n";
        sendToClient(clientFd, response);
        return;
    }

    if (targetFd == clientFd) {
        std::cerr << "Client " << clientFd << " attempted to kick themselves from channel " << channel << std::endl;
        std::string response = "482 " + channel + " :You cannot kick yourself\r\n";
        sendToClient(clientFd, response);
        return;
    }

    std::string kickReason = reason.empty() ? "No reason given" : reason;
    std::string kickMessage = ":" + it->getNickname() + " KICK " + channel + " " + target + " :" + kickReason + "\r\n";
    for (int memberFd : clientsInChannel)
    {
        sendToClient(memberFd, kickMessage);
    }

    clientsInChannel.erase(clientInChannelIt);

    std::string response = "You have been kicked from " + channel + " by " + it->getNickname() + " : " + kickReason + "\r\n";
    sendToClient(targetFd, response);

    if (clientsInChannel.empty())
    {
        channels.erase(channelIt);
        std::cout << "Channel " << channel << " is now empty and has been removed" << std::endl;
    }

    std::cout << "Client " << targetFd << " (" << targetIt->getNickname() << ") was kicked from channel " 
              << channel << " by " << it->getNickname() << " with reason: " << kickReason << std::endl;
}
