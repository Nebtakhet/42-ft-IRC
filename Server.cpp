/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/06 13:26:22 by cesasanc          #+#    #+#             */
/*   Updated: 2025/02/06 13:28:04 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port, const std::string &password) : port(port), password(password)
{
}

Server::~Server()
{
}

void	Server::run()
{
	std::cout << "Server running on port " << port << " with password " << password << std::endl;
}
