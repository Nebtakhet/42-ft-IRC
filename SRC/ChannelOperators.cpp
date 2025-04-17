/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelOperators.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 12:57:24 by cesasanc          #+#    #+#             */
/*   Updated: 2025/04/10 22:57:30 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Parsing.hpp"
#include "Commands.hpp"
#include "Client.hpp"
#include "Channel.hpp"

Channel *Server::getChannel(const std::string &channelName)
{
	auto it = channels.find(channelName);
	if (it != channels.end())
		return (&it->second);
	return (nullptr);
}

Client	*Server::getClient(int clientFd)
{
	auto it = std::find_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
		return (client.getClientFd() == clientFd);
	});
	if (it != clients.end())
		return (&(*it));
	return (nullptr);
}

Client *Server::getClientByNickname(const std::string &nickname)
{
	auto it = std::find_if(clients.begin(), clients.end(), [&nickname](const Client &client) {
		return (client.getNickname() == nickname);
	});
	if (it != clients.end())
		return (&(*it));
	return (nullptr);
}


void Server::handleKickCommand(int clientFd, const std::string &channelName, const std::string &target, const std::string &reason)
{
    Client *client = getClient(clientFd);
    if (!client)
	{
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    Channel *channel = getChannel(channelName);
    if (!channel)
	{
        std::cerr << "Channel " << channelName << " does not exist" << std::endl;
        sendToClient(clientFd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!channel->isOperator(clientFd))
	{
        std::cerr << "Client " << clientFd << " does not have permission to kick users from channel " << channelName << std::endl;
        sendToClient(clientFd, "482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client *targetClient = getClientByNickname(target);
    if (!targetClient)
	{
        std::cerr << "User " << target << " does not exist" << std::endl;
        sendToClient(clientFd, "401 " + target + " :No such nick/channel\r\n");
        return;
    }

    int targetFd = targetClient->getClientFd();
    if (!channel->isMember(targetFd))
	{
        std::cerr << "User " << target << " is not in channel " << channelName << std::endl;
        sendToClient(clientFd, "441 " + target + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }

    if (targetFd == clientFd)
	{
        std::cerr << "Client " << clientFd << " attempted to kick themselves from channel " << channelName << std::endl;
        sendToClient(clientFd, "482 " + channelName + " :You cannot kick yourself\r\n");
        return;
    }

    std::string kickReason = reason.empty() ? "No reason given" : reason;
    std::string kickMessage = ":" + client->getNickname() + " KICK " + channelName + " " + target + " :" + kickReason + "\r\n";

    for (int memberFd : channel->getMembers())
        sendToClient(memberFd, kickMessage);

    channel->removeMember(targetFd);
	targetClient->leaveChannel(channelName);
    sendToClient(targetFd, "You have been kicked from " + channelName + " by " + client->getNickname() + " : " + kickReason + "\r\n");

    if (channel->getMembers().empty())
	{
        channels.erase(channelName);
        std::cout << "Channel " << channelName << " is now empty and has been removed" << std::endl;
    }

    std::cout << "Client " << targetFd << " (" << targetClient->getNickname() << ") was kicked from channel " 
              << channelName << " by " << client->getNickname() << " with reason: " << kickReason << std::endl;
}


void Server::handleInviteCommand(int clientFd, const std::string &channelName, const std::string &target)
{
    Client *client = getClient(clientFd);
    if (!client)
	{
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    Channel *channel = getChannel(channelName);
    if (!channel)
	{
        std::cerr << "Channel " << channelName << " does not exist" << std::endl;
        sendToClient(clientFd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!channel->isOperator(clientFd))
	{
        std::cerr << "Client " << clientFd << " does not have permission to invite users to channel " << channelName << std::endl;
        sendToClient(clientFd, "482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    Client *targetClient = getClientByNickname(target);
    if (!targetClient)
	{
        std::cerr << "User " << target << " does not exist" << std::endl;
        sendToClient(clientFd, "401 " + target + " :No such nick/channel\r\n");
        return;
    }

    int targetFd = targetClient->getClientFd();
    channel->inviteUser(targetFd);

    std::string inviteMessage = ":" + client->getNickname() + " INVITE " + target + " :" + channelName + "\r\n";
    sendToClient(targetFd, inviteMessage);

    std::cout << "Client " << targetFd << " (" << targetClient->getNickname() << ") was invited to channel " 
              << channelName << " by " << client->getNickname() << std::endl;
}

void Server::handleTopicCommand(int clientFd, const std::string &channelName, const std::string &topic)
{
    Client *client = getClient(clientFd);
    if (!client)
	{
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    Channel *channel = getChannel(channelName);
    if (!channel)
	{
        std::cerr << "Channel " << channelName << " does not exist" << std::endl;
        sendToClient(clientFd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!topic.empty())
	{
        if (channel->isTopicProtected() && !channel->isOperator(clientFd))
		{
            std::cerr << "Client " << clientFd << " does not have permission to set the topic for channel " << channelName << std::endl;
            sendToClient(clientFd, "482 " + channelName + " :You're not channel operator\r\n");
            return;
        }

        channel->setTopic(topic);
        std::string topicMessage = ":" + client->getNickname() + " TOPIC " + channelName + " :" + topic + "\r\n";

        for (int memberFd : channel->getMembers())
            sendToClient(memberFd, topicMessage);


        std::cout << "Client " << clientFd << " (" << client->getNickname() << ") set topic for channel " 
                  << channelName << " to: " << topic << std::endl;
    }
	else
	{
        std::string currentTopic = channel->getTopic();
        if (!currentTopic.empty())
            sendToClient(clientFd, "332 " + channelName + " :" + currentTopic + "\r\n");
        else
            sendToClient(clientFd, "331 " + channelName + " :No topic is set\r\n");
    }
}


void Server::handleModeCommand(int clientFd, const std::string &channelName, char flag, const std::string &parameter, const std::string &arg)
{
    Client *client = getClient(clientFd);
    if (!client) 
    {
        sendToClient(clientFd, "401 :Client not found\r\n"); // ERR_NOSUCHNICK
        return;
    }

    Channel *channel = getChannel(channelName);
    if (!channel)
    {
        sendToClient(clientFd, "403 " + channelName + " :No such channel\r\n");
        return;
    }

    if (!channel->isOperator(clientFd))
    {
        sendToClient(clientFd, "482 " + channelName + " :You're not channel operator\r\n");
        return;
    }

    if (client->isOperator())
    {
        if (parameter == "i")
        {
            if (flag == '+')
                channel->setInviteOnly(true);
            else if (flag == '-')
                channel->setInviteOnly(false);
        }
        else if (parameter == "t")
        {
            if (flag == '+')
                channel->setTopicProtected(true);
            if (flag == '-')
                channel->setTopicProtected(false);
        }
        else if (parameter == "k")
        {
            if (flag == '+')
                channel->setKey(parameter);
            if (flag == '-')
                channel->clearKey();
        }
        else if (parameter == "l")
        {
            if (flag == '+')
                channel->setUserLimit(std::stoi(arg));
            if (flag == '-')
                channel->setUserLimit(1000);
        }
        else if (parameter == "o")
        {
            Client *targetClient = getClientByNickname(parameter);
            if (!targetClient)
            {
                sendToClient(clientFd, "401 " + parameter + " :No such nick/channel\r\n");
                return;
            }
            int targetFd = targetClient->getClientFd();
        
            if(flag == '+')
                channel->addOperator(targetFd);
            if (flag == '-')
                channel->removeOperator(targetFd);
        }
        else
        {
            sendToClient(clientFd, "501 :Unknown MODE flag\r\n");
            return;
        }
    }
    else
        std::cerr << "no operator privillages\r\n";
        

    std::string response = ":" + client->getNickname() + " MODE " + channelName + " " + flag + " " + parameter + "\r\n";
    for (int memberFd : channel->getMembers())
        sendToClient(memberFd, response);

    std::cout << "Client " << clientFd << " set mode " << parameter << " for channel " << channelName << std::endl;
    }   

	



void Channel::removeMember(int clientFd)
{
    members.erase(clientFd);
/*
    // Use Server's getClient function to retrieve the client
    Server *server = Server::serverInstance;
    if (!server)
    {
        std::cerr << "Server instance not found" << std::endl;
        return;
    }

    Client *client = server->getClient(clientFd);
    if (client)
    {
        // Clear the client's current channel
        client->setCurrentChannel("");
    }*/
}


/*
///////////////DUMP

        // Handle user modes (e.g., +i for invite only)
        // i: Set/remove Invite-only channel
        // t: Set/remove the restrictions of the TOPIC command to channel operators
        // k: Set/remove the channel key (password)   
        // o: Give/take channel operator privilege

        if (mode == "+i")
		{
            channel->setInviteOnly(true);
            client->addMode("i");
            std::string response = ":" + client->getNickname() + " MODE " + target + " " + mode + "\r\n";
            sendToClient(clientFd, response);
            std::cout << "Set user mode " << mode << " for client " << clientFd << std::endl;
        }
		else if (mode == "-i")
		{
            channel->setInviteOnly(false);
            client->removeMode("i");
            std::string response = ":" + client->getNickname() + " MODE " + target + " " + mode + "\r\n";
            sendToClient(clientFd, response);
            std::cout << "Removed user mode " << mode << " for client " << clientFd << std::endl;*/
        
       /* else if (mode == "-t") 
        else if (mode == "+t")
        else if (mode == "-k")
        else if (mode == "+k")
        else if (mode == "-o")
        else if (mode == "+o")
		else
            sendToClient(clientFd, "501 :Unknown MODE flag\r\n");
        return;
    }
    
    
    Channel *channel = getChannel(target);
    if (!channel)
	{
        sendToClient(clientFd, "403 " + target + " :No such channel\r\n");
        return;
    }

    if (!channel->isOperator(clientFd))
	{
        sendToClient(clientFd, "482 " + target + " :You're not channel operator\r\n");
        return;
    }

    if (mode == "+i")
        channel->setInviteOnly(true);
    else if (mode == "-i")
        channel->setInviteOnly(false);
    else if (mode == "+k")
        channel->setKey(parameter);
    else if (mode == "-k")
        channel->clearKey();
    else if (mode == "+l")
        channel->setUserLimit(std::stoi(parameter));
    else if (mode == "-l")
        channel->clearUserLimit();
    else if (mode == "+t")
        channel->setTopicProtected(true);
    else if (mode == "-t")
        channel->setTopicProtected(false);
    else if (mode == "+o" || mode == "-o")
	{
        Client *targetClient = getClientByNickname(parameter);
        if (!targetClient)
		{
            sendToClient(clientFd, "401 " + parameter + " :No such nick/channel\r\n");
            return;
        }

        int targetFd = targetClient->getClientFd();
        if (mode == "+o")
            channel->addOperator(targetFd);
        else
            channel->removeOperator(targetFd);
    }
	else
	{
        sendToClient(clientFd, "501 :Unknown MODE flag\r\n");
        return;
    }

    std::string response = ":" + client->getNickname() + " MODE " + target + " " + mode + " " + parameter + "\r\n";
    for (int memberFd : channel->getMembers())
        sendToClient(memberFd, response);

    std::cout << "Client " << clientFd << " set mode " << mode << " for channel " << target << std::endl;
*/