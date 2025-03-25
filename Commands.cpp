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