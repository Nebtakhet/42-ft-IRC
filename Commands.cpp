#include "Commands.hpp"
#include "Server.hpp"
#include <iostream>

void invite(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string targetUser = actual_cmd.message; 
    std::string channel = actual_cmd.name; 

    if (server->isUserInChannel(clientFd, channel) && server->hasInvitePermission(clientFd, channel)) {
        server->inviteUserToChannel(targetUser, channel);
        server->sendMessage(clientFd, "INVITE " + targetUser + " to " + channel);
    } else {
        server->sendMessage(clientFd, "ERROR: You don't have permission to invite users to this channel.");
    }
}

// Join a channel
void join(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string channel = actual_cmd.name; 

    server->addUserToChannel(clientFd, channel);
    server->sendMessage(clientFd, "JOIN " + channel);
}

// Kick a user from a channel
void kick(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string targetUser = actual_cmd.message; 
    std::string channel = actual_cmd.name; 

    // Check if the client has permission to kick
    if (server->hasKickPermission(clientFd, channel)) {
        server->kickUserFromChannel(targetUser, channel);
        server->sendMessage(clientFd, "KICK " + targetUser + " from " + channel);
    } else {
        server->sendMessage(clientFd, "ERROR: You don't have permission to kick users from this channel.");
    }
}

void kill(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string targetUser = actual_cmd.message; 

    if (server->hasKillPermission(clientFd)) {
        server->killUserSession(targetUser);
        server->sendMessage(clientFd, "KILL " + targetUser);
    } else {
        server->sendMessage(clientFd, "ERROR: You don't have permission to kill users.");
    }
}

void list(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string channel = actual_cmd.name; 

    std::vector<std::string> users = server->listUsersInChannel(channel);
    for (const std::string &user : users) {
        server->sendMessage(clientFd, "USER: " + user);
    }
}

void modeFunction(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string channel = actual_cmd.name; 
    std::string mode = actual_cmd.message; 

    server->setChannelMode(channel, mode);
    server->sendMessage(clientFd, "MODE " + channel + " " + mode);
}

void motd(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    (void) actual_cmd;
    std::string motdMessage = server->getMotd();
    server->sendMessage(clientFd, "MOTD: " + motdMessage);
}

void names(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string channel = actual_cmd.name; 

 
    std::vector<std::string> users = server->listUsersInChannel(channel);
    for (const std::string &user : users) {
        server->sendMessage(clientFd, "USER: " + user);
    }
}


void nick(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string newNick = actual_cmd.message; 

    server->changeNickname(clientFd, newNick);
    server->sendMessage(clientFd, "NICK " + newNick);
}


void notice(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    (void)clientFd; 
    std::string targetUser = actual_cmd.name; 
    std::string noticeMessage = actual_cmd.message; 

    server->sendNotice(targetUser, noticeMessage);
}

void oper(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string username = actual_cmd.name; 
    std::string password = actual_cmd.message; 

    if (server->authenticateOperator(username, password)) {
        server->grantOperatorPrivileges(clientFd);
        server->sendMessage(clientFd, "OPER " + username + " granted operator privileges.");
    } else {
        server->sendMessage(clientFd, "ERROR: Invalid operator credentials.");
    }
}

int pass(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string password = actual_cmd.message; // Assuming the message contains the password

    if (server->setClientPassword(clientFd, password)) {
        server->sendMessage(clientFd, "PASS command successful.");
        return 0;
    } else {
        server->sendMessage(clientFd, "ERROR: PASS command failed.");
        return 1;
    }
}

void part(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string channel = actual_cmd.name; 

    server->removeUserFromChannel(clientFd, channel);
    server->sendMessage(clientFd, "PART " + channel);
}

int ping(Server *server, int const clientFd, cmd_syntax &cmd) {
    std::string response = "PONG " + cmd.message; 
    server->sendMessage(clientFd, response);
    return 0;
}

void privmsg(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string targetUser = actual_cmd.name; 
    std::string message = actual_cmd.message; 

    server->sendPrivateMessage(clientFd, targetUser, message);
}

void quit(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string quitMessage = actual_cmd.message; 

    server->removeClient(clientFd, quitMessage);
}

void topic(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string channel = actual_cmd.name; 
    std::string topic = actual_cmd.message; 

    server->setChannelTopic(channel, topic);
    server->sendMessage(clientFd, "TOPIC " + channel + " :" + topic);
}

void user(Server *server, int const clientFd, cmd_syntax actual_cmd) {
    std::string username = actual_cmd.name; 
    std::string realname = actual_cmd.message; 

    server->registerUser(clientFd, username, realname);
    server->sendMessage(clientFd, "USER " + username + " :" + realname);
}

