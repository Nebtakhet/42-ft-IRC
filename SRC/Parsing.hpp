#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

struct cmd_syntax {
    std::string prefix;
    std::string name;
    std::vector<std::string> params;
    std::string message;
};

cmd_syntax parseIrcMessage(const std::string&);