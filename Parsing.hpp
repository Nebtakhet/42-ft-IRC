#pragma once
#include "Server.hpp"
#include "Client.hpp"
#include "Commands.hpp"
#include <iostream>
#include <sstream>

cmd_syntax parseIrcMessage(const std::string&); 