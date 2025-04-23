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

        // Acknowledge requested capabilities
        std::string response = "CAP * ACK :" + parsed.message + "\r\n";
        server->sendToClient(clientFd, response);
    } else if (subcommand == "END") {
        // Do nothing for CAP END, just proceed with registration
        std::cout << "CAP negotiation ended for client " << clientFd << std::endl;
    } else {
        std::cerr << "Unknown CAP subcommand: " << subcommand << std::endl;
        std::string response = "CAP * NAK :" + subcommand + "\r\n";
        server->sendToClient(clientFd, response);
    }
}

void join(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No channel provided for JOIN command" << std::endl;
        std::string response = "461 JOIN :Not enough parameters\r\n"; // ERR_NEEDMOREPARAMS
        server->sendToClient(clientFd, response);
        return;
    }

    std::string channel = parsed.params[0];
    std::cout << "Handling JOIN command for client " << clientFd << " with channel " << channel << std::endl;
    server->handleJoinCommand(clientFd, channel);
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
        "NICK <nickname> - Set your nickname\r\n"
        "USER <username> <hostname> <servername> :<realname> - Register your username\r\n"
        "JOIN <#channel> - Join a channel\r\n"
        "PART <#channel> - Leave a channel\r\n"
        "PRIVMSG <target> <message> - Send a private message to a user or channel\r\n"
        "PING <server> - Ping the server\r\n"
        "PASS <password> - Authenticate with the server\r\n"
        "HELP - Show this help message\r\n";

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
		return;
	}

	std::string channel = parsed.params[0];
	std::string targetNick = parsed.params[1];

	server->handleInviteCommand(clientFd, channel, targetNick);
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
