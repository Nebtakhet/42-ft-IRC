/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dbejar-s <dbejar-s@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/25 11:23:47 by dbejar-s          #+#    #+#             */
/*   Updated: 2025/04/25 11:31:42 by dbejar-s         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parsing.hpp"

cmd_syntax parseIrcMessage(const std::string& raw_msg) 
{
    cmd_syntax parsed;
    std::istringstream stream(raw_msg);

    if (raw_msg[0] == ':') 
    {
        stream >> parsed.prefix;
        parsed.prefix = parsed.prefix.substr(1);
    }
    stream >> parsed.name;

    std::string param;
    while (stream >> param) {
        if (param[0] == ':') {
            parsed.message = raw_msg.substr(raw_msg.find(param) + 1);
            break;
        } else {
            parsed.params.push_back(param);
        }
    }
    return parsed;
}
