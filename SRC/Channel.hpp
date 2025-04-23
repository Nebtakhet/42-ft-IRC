/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cesasanc <cesasanc@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/08 18:02:50 by cesasanc          #+#    #+#             */
/*   Updated: 2025/04/23 19:00:02 by cesasanc         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Client.hpp"
# include "Server.hpp"
# include <set>
# include <string>

class Channel
{
	private:
		std::string		name;
		std::set<int>	members;
		std::set<int>	operators;
		bool			inviteOnly = false;
		std::string		topic;
		bool			topicProtected = false;
		std::string		key;
		int				userLimit = 5;
		std::set<int>	invitedUsers;

	public:
		Channel(const std::string &name) : name(name) {}
		~Channel() {}
		Channel(const Channel &) = default;
		Channel &operator=(const Channel &) = default;
		
		const std::string	&getName() const { return name; }
		const std::string	&getTopic() const { return topic; }
		bool				isInviteOnly() const { return inviteOnly; }
		bool				isTopicProtected() const { return topicProtected; }
		int					getUserLimit() const { return userLimit; }

		void				addMember(int clientFd) { members.insert(clientFd); }
		void				removeMember(int clientFd);
		bool				isMember(int clientFd) const { return members.find(clientFd) != members.end(); }
		const std::set<int>	&getMembers() const { return members; }
		
		void	addOperator(int clientFd) { operators.insert(clientFd); }
		void	removeOperator(int clientFd) { operators.erase(clientFd); }
		bool	isOperator(int clientFd) const { return operators.find(clientFd) != operators.end(); }

		void	setTopic(const std::string &newTopic) { topic = newTopic; }
		void	setInviteOnly(bool inviteOnly) { this->inviteOnly = inviteOnly; }
		void	setTopicProtected(bool topicProtected) { this->topicProtected = topicProtected; }
		
		void	inviteUser(int clientFd) { invitedUsers.insert(clientFd); }
		bool	isInvited(int clientFd) const { return invitedUsers.find(clientFd) != invitedUsers.end(); }
		
		void				setKey(const std::string &newKey) { key = newKey; }
		void				clearKey() { key.clear(); }
		bool				hasKey() const { return !key.empty(); }
		const std::string	&getKey() const { return key; }

		void	setUserLimit(int limit) { userLimit = limit; }
		void	clearUserLimit() { userLimit = 0; }
		bool	userLimitReached() const { return static_cast<int>(members.size()) >= userLimit; }
		
};

#endif