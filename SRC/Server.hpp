/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:24:45 by cesasanc          #+#    #+#             */
/*   Updated: 2025/04/08 19:20:16 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <sys/socket.h>
# include <fcntl.h>
# include <netinet/in.h>
# include <unistd.h>
# include <poll.h>
# include <vector>
# include <unordered_map>
# include <chrono>
# include <iomanip>
# include <sstream>
# include <queue>
# include <csignal>
# include <algorithm>
# include "Client.hpp"
# include "Channel.hpp"
# include "Parsing.hpp"

class Server
{
	public:
		const std::string DEFAULT_CHANNEL = "#default";
		std::map<std::string, Channel> channels;

		Server(int port, const std::string &password);
		~Server();
		void setupSocket();
		void handleConnections();
		void handleClient(int clientFd);
		void removeClient(int clientFd);
		void closeServer();
		void sendMessage();
		void messageBuffer(int clientFd, const std::string &message);
		void run();
		void cleanExit();
		void handleIncomingMessage(const std::string &message, int clientFd);
		void handleNickCommand(int clientFd, const std::string &nickname);
		void handleCapLs(int clientFd);
		void handleCapReq(int clientFd, const std::vector<std::string> &capabilities);
		void handleCapEnd(int clientFd);
		void sendToClient(int clientFd, const std::string &message);
		void handleJoinCommand(int clientFd, const std::string &channel);
		void handlePartCommand(int clientFd, const std::string &channel, const cmd_syntax &parsed);
		void handlePrivmsgCommand(int clientFd, const std::string &target, const std::string &message);
		void handleHelpCommand(int clientFd);
		void handleWhoCommand(int clientFd, const std::string &target);
		void handleQuitCommand(int clientFd, const std::string &quitMessage);
		void handleUserCommand(int clientFd, const std::string &username, const std::string &hostname, const std::string &servername, const std::string &realname);
		void handlePassCommand(int clientFd, const std::string &password);

		void handleKickCommand(int clientFd, const std::string &channel, const std::string &target, const std::string &reason);
		void handleInviteCommand(int clientFd, const std::string &channel, const std::string &target);
		void handleTopicCommand(int clientFd, const std::string &channel, const std::string &topic);		
		void handleModeCommand(int clientFd, const std::string &channelName, char flag, const std::string &parameter, const std::string &arg);
		
		void sendWelcomeMessage(int clientFd, const Client &client);

		Channel	*getChannel(const std::string &channelName);
		Client	*getClient(int clientFd);
		Client	*getClientByNickname(const std::string &nickname);
		
	private:
		int port;
		std::string password;
		int serverSocket;
		bool running;
		struct sockaddr_in serverAddress;
		std::vector<struct pollfd> pollfds;
		std::unordered_map<int, std::string> clientBuffer;
		std::vector<Client> clients;
};

extern Server *serverInstance;

#endif