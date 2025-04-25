/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsing.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbejar-s <dbejar-s@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 11:22:09 by dbejar-s          #+#    #+#             */
/*   Updated: 2025/04/25 11:22:09 by dbejar-s         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSING_HPP
# define PARSING_HPP

# include <iostream>
# include <sstream>
# include <vector>
# include <string>

struct cmd_syntax {
    std::string prefix;
    std::string name;
    std::vector<std::string> params;
    std::string message;
};

cmd_syntax parseIrcMessage(const std::string&);

#endif