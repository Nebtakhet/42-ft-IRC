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
        server->handleCapLs(clientFd);
    } else if (subcommand == "REQ") {
        std::vector<std::string> capabilities;
        std::istringstream iss(parsed.message);
        std::string cap;
        while (iss >> cap) {
            capabilities.push_back(cap);
        }
        server->handleCapReq(clientFd, capabilities);
    } else if (subcommand == "END") {
        server->handleCapEnd(clientFd);
    } else {
        std::cerr << "Unknown CAP subcommand: " << subcommand << std::endl;
        std::string response = "CAP * NAK :" + subcommand + "\r\n";
        server->sendToClient(clientFd, response);
    }
}

void join(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.empty()) {
        std::cerr << "No channel provided" << std::endl;
        return;
    }

    std::string channel = parsed.params[0];
    server->handleJoinCommand(clientFd, channel);
}

void user(Server *server, int clientFd, const cmd_syntax &parsed) {
    if (parsed.params.size() < 4) {
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