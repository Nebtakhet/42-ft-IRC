#include "Commands.hpp"

void nick(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No nickname provided" << std::endl;
        return;
    }

    std::string nickname = parsed.params[0];
    server->handleNickCommand(clientFd, nickname);
}

void cap(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No CAP command provided" << std::endl;
        return;
    }

    std::string subcommand = parsed.params[0];
    if (subcommand == "LS") {
        std::string capList = "multi-prefix sasl";
        std::string response = "CAP * LS :" + capList + "\r\n";
        server->sendToClient(clientFd, response);
    } else if (subcommand == "REQ") {
        if (parsed.message.empty()) {
            std::cerr << "No capabilities requested" << std::endl;
            return;
        }

        std::string response = "CAP * ACK :" + parsed.message + "\r\n";
        server->sendToClient(clientFd, response);
    } else if (subcommand == "END") {
		Client *client = server->getClient(clientFd);
		if (!client) {
			std::cerr << "Client " << clientFd << " not found" << std::endl;
			return;
		}
		client->setCapNegotiation(false);

        std::cout << "CAP negotiation ended for client " << clientFd << std::endl;
    } else {
        std::cerr << "Unknown CAP subcommand: " << subcommand << std::endl;
        std::string response = "CAP * NAK :" + subcommand + "\r\n";
        server->sendToClient(clientFd, response);
    }
}

void join(Server *server, int clientFd, const cmd_syntax &parsed) {
    Client *client = server->getClient(clientFd);
    if (!client) {
        std::cerr << "Client " << clientFd << " not found" << std::endl;
        return;
    }

    if (client->isCapNegotiating()) {
        std::cerr << "Client " << clientFd << " attempted to JOIN during CAP negotiation" << std::endl;

        if (parsed.params.empty() || parsed.params[0].empty()) {
            std::cout << "Ignoring empty JOIN command from client " << clientFd << " during CAP negotiation" << std::endl;
            return;
        }

        std::string response = "451 JOIN :You cannot join a channel during CAP negotiation\r\n"; // ERR_NOTREGISTERED
        server->sendToClient(clientFd, response);
        return;
    }

    if (parsed.params.empty() || parsed.params[0].empty()) {
        std::cerr << "No channel provided for JOIN command from client " << clientFd << std::endl;
        std::string response = "461 JOIN :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        server->sendToClient(clientFd, response);
        return;
    }

    std::string channel = parsed.params[0];
    std::string providedKey = parsed.params.size() > 1 ? parsed.params[1] : "";
    std::cout << "Handling JOIN command for client " << clientFd << " with channel " << channel << std::endl;
    server->handleJoinCommand(clientFd, channel, providedKey);
}

void user(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.size() < 3 || parsed.message.empty()) {
        std::cerr << "Not enough parameters for USER command" << std::endl;
        return;
    }

    std::string username = parsed.params[0];
    std::string hostname = parsed.params[1];
    std::string servername = parsed.params[2];
    std::string realname = parsed.message;

    server->handleUserCommand(clientFd, username, hostname, servername, realname);
}

void pass(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No password provided" << std::endl;
        return;
    }

    std::string password = parsed.params[0];
    server->handlePassCommand(clientFd, password);
}

void ping(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No PING parameters provided" << std::endl;
        return;
    }

    std::string response = "PONG :" + parsed.params[0] + "\r\n";
    server->sendToClient(clientFd, response);
}

void part(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No channel provided for PART command" << std::endl;
        std::string response = "461 PART :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        server->sendToClient(clientFd, response);
        return;
    }

    std::string channel = parsed.params[0];
    server->handlePartCommand(clientFd, channel, parsed);
}

void privmsg(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty() || parsed.message.empty()) {
        std::cerr << "No target or message provided for PRIVMSG command" << std::endl;
        std::string response = "461 PRIVMSG :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        server->sendToClient(clientFd, response);
        return;
    }

    std::string target = parsed.params[0];
    std::string message = parsed.message;

    server->handlePrivmsgCommand(clientFd, target, message);
}

void help(Server *server, int clientFd, const cmd_syntax &parsed) {
    (void)parsed; // The HELP command does not require parameters

    std::string response =
        "Available commands:\r\n"
        "NICK nickname - Set your nickname\r\n"
        "JOIN channel - Join a channel\r\n"
        "PART channel - Leave a channel\r\n"
        "KICK channel nickname - Kick a user from a channel\r\n"
        "TOPIC channel topic - Set the topic of a channel\r\n"
        "INVITE channel nickname - Invite a user to a channel\r\n"
        "PRIVMSG target message - Send a private message to a user or channel\r\n"
        "PASS password - Authenticate with the server\r\n"
        "MODE channel mode - Set the mode of a channel\r\n"
        "MODE flags:\r\n"
        "    +i - Invite only\r\n"
        "    -i - Not invite only\r\n"
        "    +k key - Set a key for the channel\r\n"
        "    -k key - Remove the key from the channel\r\n"
        "    +l limit - Set a limit for the number of users in the channel\r\n"
        "    -l - Remove the limit from the channel\r\n"
        "    +o nickname - Give operator status to a user\r\n"
        "    -o nickname - Remove operator status from a user\r\n"
        "    +t - Topic protected\r\n"
        "    -t - Topic not protected\r\n"
        "HELP - Show this help message;\r\n";

    server->sendToClient(clientFd, response);
}

void who(Server *server, int clientFd, const cmd_syntax &parsed) {
    std::string target = parsed.params.empty() ? "" : parsed.params[0];

    server->handleWhoCommand(clientFd, target);
}

void quit(Server *server, int clientFd, const cmd_syntax &parsed) {
    std::string quitMessage = parsed.message.empty() ? "Client disconnected" : parsed.message;

    server->handleQuitCommand(clientFd, quitMessage);
}

void kick(Server *server, int clientFd, const cmd_syntax &parsed) 
{
	if (parsed.params.size() < 2) {
		std::cerr << "Not enough parameters for KICK command" << std::endl;
		return;
	}

	std::string channel = parsed.params[0];
	std::string targetNick = parsed.params[1];
	std::string reason = parsed.message.empty() ? "No reason provided" : parsed.message;

	server->handleKickCommand(clientFd, channel, targetNick, reason);
}

void invite(Server *server, int clientFd, const cmd_syntax &parsed) 
{
    if (parsed.params.size() < 2) {
        std::cerr << "Not enough parameters for INVITE command" << std::endl;
        std::string response = "461 INVITE :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        server->sendToClient(clientFd, response);
        return;
    }

    std::string channelName = parsed.params[0];
    std::string targetNick = parsed.params[1];

    // Retrieve the inviting client
    Client *client = server->getClient(clientFd);
    if (!client) {
        std::cerr << "Inviting client not found" << std::endl;
        return;
    }

    // Retrieve the target client
    Client *targetClient = server->getClientByNickname(targetNick);
    if (!targetClient) {
        std::cerr << "Target client " << targetNick << " not found" << std::endl;
        std::string response = "401 " + targetNick + " :No such nick/channel\r\n"; // ERR_NOSUCHNICK
        server->sendToClient(clientFd, response);
        return;
    }

    // Retrieve the channel
    Channel *channel = server->getChannel(channelName);
    if (!channel) {
        std::cerr << "Channel " << channelName << " does not exist" << std::endl;
        std::string response = "403 " + channelName + " :No such channel\r\n"; // ERR_NOSUCHCHANNEL
        server->sendToClient(clientFd, response);
        return;
    }

    // Check if the inviting client is an operator in the channel
    if (!channel->isOperator(clientFd)) {
        std::cerr << "Client " << clientFd << " is not an operator in channel " << channelName << std::endl;
        std::string response = "482 " + channelName + " :You're not channel operator\r\n"; // ERR_CHANOPRIVSNEEDED
        server->sendToClient(clientFd, response);
        return;
    }

    // Invite the target client to the channel
    int targetFd = targetClient->getClientFd();
    channel->inviteUser(targetFd);

    // Send the invite message to the target client
    std::string inviteMessage = ":" + client->getNickname() + "!" + 
        client->getUsername() + "@" + server->getHostname() + " INVITE " + 
        targetNick + " :" + channelName + "\r\n";
    server->sendToClient(targetFd, inviteMessage);

    std::cout << "Client " << targetFd << " (" << targetClient->getNickname() << ") was invited to channel " 
              << channelName << " by " << client->getNickname() << std::endl;
}

void topic(Server *server, int clientFd, const cmd_syntax &parsed) 
{
	if (parsed.params.empty()) {
		std::cerr << "No channel provided for TOPIC command" << std::endl;
		return;
	}

	std::string channel = parsed.params[0];
	std::string topic = parsed.message;

	server->handleTopicCommand(clientFd, channel, topic);
}

// With my small brain i can only think of how to handle one mode at a time
void mode(Server *server, int clientFd, const cmd_syntax &parsed) 
{
    if (parsed.params.size() < 2) 
    {
        std::cerr << "Not enough parameters for MODE command" << std::endl;
        std::string response = "461 MODE :Not enough parameters\r\n"; 
        server->sendToClient(clientFd, response);
        return;
    }

    std::string channelName = parsed.params[0];
    std::string modeString = parsed.params[1];
    size_t paramIndex = 2;
    char currentFlag = '\0';

    for (char modeChar : modeString) 
    {
        if (modeChar == '+' || modeChar == '-') 
        {
            currentFlag = modeChar;
        } 
        else 
        {
            std::string parameter;

            if ((modeChar == 'k' || modeChar == 'o' || modeChar == 'l') && paramIndex < parsed.params.size()) 
            {
                parameter = parsed.params[paramIndex];
                paramIndex++;
            } 
            else if (modeChar == 'k' || modeChar == 'o' || modeChar == 'l') 
            {
                std::cerr << "Not enough parameters for MODE command" << std::endl;
                std::string response = "461 MODE :Not enough parameters\r\n"; 
                server->sendToClient(clientFd, response);
                return;
            }

            server->handleModeCommand(clientFd, channelName, currentFlag, modeChar, parameter);
        }
    }

    if (paramIndex < parsed.params.size()) 
    {
        std::cerr << "Extra parameters provided for MODE command" << std::endl;
    }
}
