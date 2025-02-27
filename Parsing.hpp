#pragma once
#include "Server.hpp"
#include "Client.hpp"
#include "Commands.hpp"
#include <iostream>
#include <sstream>

cmd_syntax parse_irc_message(const std::string&); 