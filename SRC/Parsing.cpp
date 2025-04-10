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
            parsed.message = param.substr(1);
            std::getline(stream, param);
            parsed.message += param;
            break;
        } else {
            parsed.params.push_back(param);
        }
    }

   // Handle cases where the message is not prefixed with a colon
    if (parsed.name == "PRIVMSG" && parsed.params.size() > 1 && parsed.message.empty()) {
        parsed.message = parsed.params.back();
        parsed.params.pop_back();
    }

    std::cout << "Prefix: " << parsed.prefix << "\nName: " << parsed.name << "\nParams: ";
    for (const auto& p : parsed.params) {
        std::cout << p << " ";
    }
    std::cout << "\nMessage: " << parsed.message << "\n\n";

    return parsed;
}
