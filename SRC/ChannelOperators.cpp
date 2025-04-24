/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ChannelOperators.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/07 12:57:24 by cesasanc          #+#    #+#             */
/*   Updated: 2025/04/15 00:19:15 by cesasanc         ###   ########.fr       */
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
    {
		sendToClient(memberFd, kickMessage);
	}

		
	sendToClient(targetFd, "You have been kicked from " + channelName + " by " + client->getNickname() + " : " + kickReason + "\r\n");
    channel->removeMember(targetFd);
	targetClient->leaveChannel(channelName);

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


void Server::handleModeCommand(int clientFd, const std::string &channelName, char currentFlag, char modeChar, const std::string &parameter)
{
    Client *client = getClient(clientFd);
    if (!client) 
    {
        sendToClient(clientFd, "401 :Client not found\r\n");
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

    if (modeChar == 'i') 
    {
        channel->setInviteOnly(currentFlag == '+');
    } 
    else if (modeChar == 't') 
    {
        channel->setTopicProtected(currentFlag == '+');
    } 
    else if (modeChar == 'k') 
    {
        if (currentFlag == '+') 
        {
            channel->setKey(parameter);
        } 
        else 
        {
            channel->clearKey();
        }
    } 
    else if (modeChar == 'l') 
    {
        if (currentFlag == '+') 
        {
            try 
            {
                channel->setUserLimit(std::stoi(parameter));
            } 
            catch (const std::exception &e) 
            {
                sendToClient(clientFd, "461 MODE :Invalid parameter for +l\r\n");
                return;
            }
        } 
        else 
        {
            std::cout<<"in here\n";
            channel->clearUserLimit();
        }
    } 
    else if (modeChar == 'o') 
    {
        Client *targetClient = getClientByNickname(parameter);
        if (!targetClient) 
        {
            sendToClient(clientFd, "401 " + parameter + " :No such nick/channel\r\n");
            return;
        }

        int targetFd = targetClient->getClientFd();
        if (currentFlag == '+') 
        {
            channel->addOperator(targetFd);
        } 
        else 
        {
            channel->removeOperator(targetFd);
        }
    } 
    else 
    {
        sendToClient(clientFd, "501 :Unknown MODE flag\r\n");
        return;
    }

    // Notify all channel members about the mode change
    std::string response = ":" + client->getNickname() + " MODE " + channelName + " " + currentFlag + modeChar + " " + parameter + "\r\n";
    for (int memberFd : channel->getMembers()) 
    {
        sendToClient(memberFd, response);
    }

    std::cout << "Client " << clientFd << " set mode " << currentFlag << modeChar << " for channel " << channelName << std::endl;
}

void Channel::removeMember(int clientFd)
{
    members.erase(clientFd);
}

