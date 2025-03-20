/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:24:45 by cesasanc          #+#    #+#             */
/*   Updated: 2025/03/06 11:19:23 by cesasanc         ###   ########.fr       */
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
# include <queue>

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();
		
		void	run();
		void	messageBuffer(int clientFd, const std::string &message);
		void	cleanExit();

	private:
		int										port;
		std::string								password;
		int										serverSocket;
		struct sockaddr_in						serverAddress;
		std::vector<struct pollfd>				pollfds;
		std::unordered_map<int, std::string>	clientBuffer;
		bool									running;
		

		void	setupSocket();
		void	handleConnections();
		void	handleClient(int clientFd);
		void	removeClient(int clientFd);
		void	sendMessage();
		void	closeServer();
		void 	handleIncomingMessage(const std::string&, int clientFd);
};

extern	Server	*serverInstance;

#endif