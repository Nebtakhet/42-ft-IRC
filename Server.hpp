/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:24:45 by cesasanc          #+#    #+#             */
/*   Updated: 2025/02/11 15:26:37 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <fcntl.h>
# include <netinet/in.h>

class Server
{
	public:
		Server(int port, const std::string &password);
		~Server();
		
		void	run();

	private:
		int				port;
		std::string		password;
		int					serverSocket;
		struct sockaddr_in serverAddress;

		void	setupSocket();
		void	closeServer();

};

#endif