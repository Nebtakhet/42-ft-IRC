#include "Parsing.hpp"


// this fuction should work if the raw_msg is correct, no bad inputs have been taken into account

cmd_syntax parse_irc_message(const std::string& raw_msg) 
{
	//raw_msg = ":Leo PRIVMSG #channel :Hello, world!\r\n"

    cmd_syntax parsed;
    std::istringstream stream(raw_msg); // this is used for parsing, the ">>" operator is used for extracting  
 	                                    // also i string delimits the string by ' ' automatically
    if (raw_msg[0] == ':') 
	{
        stream >> parsed.prefix;  
        parsed.prefix = parsed.prefix.substr(1);  
    
    stream >> parsed.name;

    std::getline(stream, parsed.message); // using getline to take us to the end of the message
    if (!parsed.message.empty() && parsed.message[1] == ':') 
        parsed.message = parsed.message.substr(2); 

	std::cout << "Prefix: " << parsed.prefix << "\nName: " << parsed.name << "\nMessage: " << parsed.message << "\n\n";

    return parsed;
}
