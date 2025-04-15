#include "Parsing.hpp"

cmd_syntax parseIrcMessage(const std::string& raw_msg) 
{
    cmd_syntax parsed;
    std::istringstream stream(raw_msg);

    if (raw_msg[0] == ':') 
    {
        stream >> parsed.prefix;
        parsed.prefix = parsed.prefix.substr(1); // Remove the leading ':'
    }
    stream >> parsed.name;

    std::string param;
    while (stream >> param) {
        if (param[0] == ':') {
            // The rest of the line is the message
            parsed.message = raw_msg.substr(raw_msg.find(param) + 1);
            break;
        } else {
            parsed.params.push_back(param);
        }
    }

    return parsed;
}
