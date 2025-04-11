#include "Client.hpp"

Client::Client(int clientFd) : _clientFd(clientFd), _authenticated(false), _capNegotiation(true) {} // Initialize _authenticated

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

const std::string& Client::getNickname() const { 
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

void Client::addMode(const std::string &mode) {
    if (_mode.find(mode) == std::string::npos) {
        _mode += mode;
    }
}

void Client::removeMode(const std::string &mode) {
    size_t pos = _mode.find(mode);
    if (pos != std::string::npos) {
        _mode.erase(pos, mode.length());
    }
}

void Client::addCapability(const std::string &capability) {
    if (std::find(_capabilities.begin(), _capabilities.end(), capability) == _capabilities.end()) {
        _capabilities.push_back(capability);
    }
}

bool Client::hasCapability(const std::string &capability) const {
    return std::find(_capabilities.begin(), _capabilities.end(), capability) != _capabilities.end();
}

void Client::clearCapabilities() {
    _capabilities.clear();
}

void Client::setAuthenticated(bool authenticated) {
    _authenticated = authenticated;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

void Client::setCapNegotiation(bool capNegotiation) {
    _capNegotiation = capNegotiation;
}

bool Client::isCapNegotiating() const {
    return _capNegotiation;
}