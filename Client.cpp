#include "Client.hpp"

Client::Client(int clientFd) : _clientFd(clientFd) {}

Client::~Client() {}

int Client::getClientFd() const {
    return _clientFd;
}

void Client::setNickname(const std::string &nickname) {
    _nickname = nickname;
}

std::string& Client::getReadBuffer() {
    return _read_buffer;
}

void Client::setReadBuffer(const std::string &buf) {
    _read_buffer = buf;
}

std::string& Client::getSendBuffer() {
    return _send_buffer;
}

void Client::setSendBuffer(const std::string &buf) {
    _send_buffer = buf;
}

std::string& Client::getNickname() {
    return _nickname;
}

void Client::setOldNickname(const std::string &nickname) {
    _old_nickname = nickname;
}

std::string& Client::getOldNickname() {
    return _old_nickname;
}

void Client::setUsername(const std::string &username) {
    _username = username;
}

std::string Client::getUsername() const {
    return _username;
}

void Client::setRealname(const std::string &realname) {
    _realname = realname;
}

std::string Client::getRealname() const {
    return _realname;
}

std::string& Client::getMode() {
    return _mode;
}

void Client::addMode(std::string mode) {
    if (_mode.find(mode) == std::string::npos) {
        _mode += mode;
    }
}

void Client::removeMode(std::string mode) {
    size_t pos = _mode.find(mode);
    if (pos != std::string::npos) {
        _mode.erase(pos, mode.length());
    }
}