/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/04/09 12:13:23 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Parsing.hpp"
#include "Commands.hpp"
#include "Client.hpp"
#include "Channel.hpp"

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
    cmd_syntax parsed = parseIrcMessage(message);

    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end() && it->isCapNegotiating()) {
        // Allow all implemented commands during CAP negotiation
        if (parsed.name != "CAP" && parsed.name != "PASS" && parsed.name != "NICK" && parsed.name != "USER" &&
            parsed.name != "JOIN" && parsed.name != "PART" && parsed.name != "PRIVMSG" && parsed.name != "PING" &&
            parsed.name != "QUIT" && parsed.name != "HELP" && parsed.name != "WHO" && parsed.name != "KICK" &&
            parsed.name != "INVITE" && parsed.name != "TOPIC" && parsed.name != "MODE") {
            std::cerr << "Ignoring command " << parsed.name << " during CAP negotiation for client " << clientFd << std::endl;
            return;
        }
    }

    // Process the command as usual
    if (parsed.name == "CAP")
        cap(this, clientFd, parsed);
    else if (parsed.name == "PASS")
        pass(this, clientFd, parsed);
    else if (parsed.name == "NICK")
        nick(this, clientFd, parsed);
    else if (parsed.name == "USER")
        user(this, clientFd, parsed);
    else if (parsed.name == "JOIN")
        join(this, clientFd, parsed);
    else if (parsed.name == "PART")
        part(this, clientFd, parsed);
    else if (parsed.name == "PRIVMSG")
        privmsg(this, clientFd, parsed);
    else if (parsed.name == "PING")
        ping(this, clientFd, parsed);
    else if (parsed.name == "QUIT")
        quit(this, clientFd, parsed);
    else if (parsed.name == "HELP")
        help(this, clientFd, parsed);
    else if (parsed.name == "WHO")
        who(this, clientFd, parsed);
	else if (parsed.name == "KICK")
		kick(this, clientFd, parsed);
	else if (parsed.name == "INVITE")
		invite(this, clientFd, parsed);
	else if (parsed.name == "TOPIC")
		topic(this, clientFd, parsed);
    else if (parsed.name == "MODE")
		mode(this, clientFd, parsed);
	else
        std::cerr << "Unknown command: " << parsed.name << std::endl;


    // Check if the client is fully registered
    if (it != clients.end() && it->isAuthenticated() && !it->getNickname().empty() && !it->getUsername().empty() && !it->isWelcomeSent())
	{
        sendWelcomeMessage(clientFd, *it);
        it->setWelcomeSent(true); // Mark the welcome message as sent
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
    auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    });

    if (it != clients.end()) {
        it->setCapNegotiation(false); // Mark CAP negotiation as complete
        std::cout << "CAP negotiation ended for client " << clientFd << std::endl;
    }

    // No need to send a CAP END response; that was the damn mistake, motherfucker!!!!
}

void Server::handleJoinCommand(int clientFd, const std::string &channelName)
{
    if (channelName.empty())
	{
        std::cerr << "No channel provided for JOIN command" << std::endl;
        std::string response = "461 JOIN :Not enough parameters\r\n";
        sendToClient(clientFd, response);
        return;
    }

    Client *client = getClient(clientFd);
    if (!client)
	{
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    Channel *channel = getChannel(channelName);
    if (!channel)
	{
        auto result = channels.emplace(channelName, Channel(channelName));
        channel = &result.first->second;

        channel->addOperator(clientFd);
        std::cout << "Channel " << channelName << " created and client " << clientFd << " set as operator" << std::endl;
    }

    if (channel->isMember(clientFd))
	{
        std::cerr << "Client " << clientFd << " is already in channel " << channelName << std::endl;
        return;
    }

    channel->addMember(clientFd);
    std::cout << "Added client " << clientFd << " to channel " << channelName << std::endl;

    std::string response = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
    sendToClient(clientFd, response);

    for (int memberFd : channel->getMembers())
	{
        if (memberFd != clientFd)
            sendToClient(memberFd, response);
    }

    std::cout << "Client " << clientFd << " joined channel " << channelName << std::endl;
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

void Server::handlePartCommand(int clientFd, const std::string &channelName)
{
    Channel *channel = getChannel(channelName);
    if (!channel) {
        std::cerr << "Channel " << channelName << " does not exist" << std::endl;
        std::string response = "403 " + channelName + " :No such channel\r\n"; 
        sendToClient(clientFd, response);
        return;
    }

    if (!channel->isMember(clientFd)) {
        std::cerr << "Client " << clientFd << " is not in channel " << channelName << std::endl;
        std::string response = "442 " + channelName + " :You're not on that channel\r\n";
        sendToClient(clientFd, response);
        return;
    }

    channel->removeMember(clientFd);
    std::cout << "Client " << clientFd << " left channel " << channelName << std::endl;

    std::string response = ":" + getClient(clientFd)->getNickname() + " PART " + channelName + "\r\n";
    sendToClient(clientFd, response);

    for (int memberFd : channel->getMembers()) {
        sendToClient(memberFd, response);
    }

    if (channel->getMembers().empty()) {
        channels.erase(channelName);
        std::cout << "Channel " << channelName << " is now empty and has been removed" << std::endl;
    }
}

void Server::handlePrivmsgCommand(int clientFd, const std::string &target, const std::string &message)
{
    Client *client = getClient(clientFd);
    if (!client)
	{
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    std::string sender = client->getNickname();

    // Check if the target is a channel
    if (target[0] == '#')
	{
        Channel *channel = getChannel(target);
        if (!channel)
		{
            std::cerr << "Channel " << target << " does not exist" << std::endl;
            std::string response = "403 " + target + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
            sendToClient(clientFd, response);
            return;
        }

        // Check if the client is a member of the channel
        if (!channel->isMember(clientFd))
		{
            std::cerr << "Client " << clientFd << " is not a member of channel " << target << std::endl;
            std::string response = "442 " + target + " :You're not on that channel\r\n"; // ERR_NOTONCHANNEL
            sendToClient(clientFd, response);
            return;
        }

        // Send the message to all clients in the channel except the sender
        for (int memberFd : channel->getMembers())
		{
            if (memberFd != clientFd)
			{
                std::string response = ":" + sender + " PRIVMSG " + target + " :" + message + "\r\n";
                sendToClient(memberFd, response);
                std::cout << "Sent message to client " << memberFd << " in channel " << target << std::endl;
            }
        }
    } else
	{
        // Target is a user
        Client *targetClient = getClientByNickname(target);
        if (!targetClient)
		{
            std::cerr << "User " << target << " does not exist" << std::endl;
            std::string response = "401 " + target + " :No such nick/channel\r\n"; // ERR_NOSUCHNICK
            sendToClient(clientFd, response);
            return;
        }

        // Send the message to the target user
        int targetFd = targetClient->getClientFd();
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

void Server::handleWhoCommand(int clientFd, const std::string &target)
{
    std::ostringstream response;

    if (target.empty())
	{
        // WHO without a target: list all users on the server
        for (const Client &client : clients) {
            response << client.getNickname() << " "
                     << client.getUsername() << " "
                     << client.getRealname() << "\r\n";
        }
    }
	else if (target[0] == '#')
	{
        // WHO for a channel
        Channel *channel = getChannel(target);
        if (!channel)
		{
            std::cerr << "Channel " << target << " does not exist" << std::endl;
            std::string errorResponse = "403 " + target + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
            sendToClient(clientFd, errorResponse);
            return;
        }

        for (int memberFd : channel->getMembers())
		{
            Client *member = getClient(memberFd);
            if (member)
			{
                response << member->getNickname() << " "
                         << member->getUsername() << " "
                         << member->getRealname() << "\r\n";
            }
        }
    }
	else
	{
        // WHO for a specific user
        Client *targetClient = getClientByNickname(target);
        if (!targetClient)
		{
            std::cerr << "User " << target << " does not exist" << std::endl;
            std::string errorResponse = "401 " + target + " :No such nick/channel\r\n"; // ERR_NOSUCHNICK
            sendToClient(clientFd, errorResponse);
            return;
        }

        response << targetClient->getNickname() << " "
                 << targetClient->getUsername() << " "
                 << targetClient->getRealname() << "\r\n";
    }

    sendToClient(clientFd, response.str());
}

void Server::handleQuitCommand(int clientFd, const std::string &quitMessage)
{
    Client *client = getClient(clientFd);
    if (!client)
	{
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    std::string nickname = client->getNickname();

    for (auto &channelPair : channels)
	{
        Channel &channel = channelPair.second;
        if (channel.isMember(clientFd))
		{
            std::string response = ":" + nickname + " QUIT :" + quitMessage + "\r\n";
            for (int memberFd : channel.getMembers())
			{
                if (memberFd != clientFd)
                    sendToClient(memberFd, response);
            }
        }
    }

    for (auto it = channels.begin(); it != channels.end();)
	{
        Channel &channel = it->second;
        channel.removeMember(clientFd);

        if (channel.getMembers().empty())
            it = channels.erase(it);
        else
            ++it;
    }

    std::cout << "Client " << clientFd << " (" << nickname << ") disconnected with message: " << quitMessage << std::endl;
    removeClient(clientFd);
}

void Server::sendWelcomeMessage(int clientFd, const Client &client) {
    std::string welcomeMessage = "001 " + client.getNickname() + " :Welcome to the Internet Relay Network " + client.getNickname() + "!" + client.getUsername() + "@localhost\r\n";
    sendToClient(clientFd, welcomeMessage);

    std::cout << "Sent welcome message to client " << clientFd << std::endl;
}
