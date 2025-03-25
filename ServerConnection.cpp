#include "Server.hpp"
#include "Parsing.hpp"
#include "Commands.hpp"

/* Function to setup the socket. It creates a socket, sets it to non-blocking, 
binds it to the server address and listens on it. It also adds the server socket
to the pollfds vector. */
void Server::setupSocket()
{
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        exit(EXIT_FAILURE);
    }

    if (fcntl(serverSocket, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << "Failed to set server socket to non-blocking" << std::endl;
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Failed to set socket options" << std::endl;
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to bind socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 10) == -1)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    pollfds.push_back({serverSocket, POLLIN, 0});
    std::cout << "Socket setup complete. Listening on port " << port << std::endl;
}

/* Function to handle incoming connections. It accepts the connection, sets the client
socket to non-blocking and adds it to the pollfds vector. */
void Server::handleConnections()
{
    struct sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientFd;

    while ((clientFd = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen)) >= 0)
    {
        if (fcntl(clientFd, F_SETFL, O_NONBLOCK) == -1)
        {
            std::cerr << "Failed to set client socket to non-blocking" << std::endl;
            close(clientFd);
            continue;
        }
        pollfds.push_back({clientFd, POLLIN, 0});
        clients.emplace_back(clientFd); 
        std::cout << "New client connected: " << clientFd << std::endl;

        // Automatically join the default channel
        handleJoinCommand(clientFd, DEFAULT_CHANNEL);
    }

    if (errno == EWOULDBLOCK || errno == EAGAIN)
        return;
    std::cerr << "Failed to accept client connection: " << strerror(errno) << std::endl;
}

/* Function to handle incoming messages from clients. It reads the message, prints it
to the console and adds it to the clientBuffer. If the client disconnects, it removes
the client from the pollfds vector and closes the connection. */
void Server::handleClient(int clientFd)
{
    char buffer[512];
    int bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';
        clientBuffer[clientFd] += std::string(buffer, bytesRead);

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm *now_tm = std::localtime(&now_time);

        std::ostringstream time_stream;
        time_stream << std::put_time(now_tm, "%Y-%m-%d %H:%M:%S");

        // DEBUGGING
        std::cout << time_stream.str() << " >>>>>>>>>>>>>>>>>>>> Received from client " << clientFd << " >>> " << buffer << std::endl;

        size_t pos;
        while ((pos = clientBuffer[clientFd].find('\n')) != std::string::npos)
        {
            std::string command = clientBuffer[clientFd].substr(0, pos);
            clientBuffer[clientFd].erase(0, pos + 1);

            std::cout << "Client " << clientFd << ": " << command << std::endl;
            messageBuffer(clientFd, command);
        }

        if (clientBuffer[clientFd].find('\0') != std::string::npos)
        {
            std::string command = clientBuffer[clientFd];
            clientBuffer[clientFd].clear();

            std::cout << "Client " << clientFd << ": " << command << std::endl;
            messageBuffer(clientFd, command);
        }
        handleIncomingMessage(std::string(buffer, bytesRead), clientFd);
    }
    else if (bytesRead == 0)
    {
        std::cout << "Client " << clientFd << " disconnected" << std::endl;
        removeClient(clientFd);
    }
    else if (bytesRead == -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return;
        std::cerr << "Failed to receive message from " << clientFd << ": " << strerror(errno) << std::endl;
        removeClient(clientFd);
    }
}

/* Function to remove a client from the pollfds vector and close the connection. */
void Server::removeClient(int clientFd)
{
    close(clientFd);
    for (size_t i = pollfds.size(); i-- > 0; )
    {
        if (pollfds[i].fd == clientFd)
        {
            pollfds.erase(pollfds.begin() + i);
            break;
        }
    }
    clientBuffer.erase(clientFd);

    // Remove client from clients container
    clients.erase(std::remove_if(clients.begin(), clients.end(), [clientFd](const Client &client) {
        return client.getClientFd() == clientFd;
    }), clients.end());

    for (auto &channel : channels)
    {
        channel.second.erase(std::remove(channel.second.begin(), channel.second.end(), clientFd), channel.second.end());
    }
}

/* Function to close the server. It closes all client connections and the server socket. */
void Server::closeServer()
{
    for (size_t i = 0; i < pollfds.size(); i++)
    {
        if (pollfds[i].fd != serverSocket)
            close(pollfds[i].fd);
    }
    close(serverSocket);
    pollfds.clear();
    clientBuffer.clear();
    running = false;
}

/* Function to send messages to clients. It iterates through the pollfds vector and
sends messages to clients that have messages in the clientBuffer. */
void Server::sendMessage()
{
    for (size_t i = pollfds.size(); i-- > 0;)
    {
        if (pollfds[i].revents & POLLOUT)
        {
            int clientFd = pollfds[i].fd;
            if (clientBuffer.find(clientFd) != clientBuffer.end() && !clientBuffer[clientFd].empty())
            {
                std::string &message = clientBuffer[clientFd];

                ssize_t bytesSent = send(clientFd, message.c_str(), message.size(), 0);
                if (bytesSent > 0)
                {
                    message.erase(0, bytesSent);
                    if (message.empty())
                    {
                        clientBuffer[clientFd].clear();
                        if (clientBuffer[clientFd].empty())
                            pollfds[i].events &= ~POLLOUT;
                    }
                }
                else if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    std::cerr << "Failed to send message to " << clientFd << std::endl;
                    removeClient(clientFd);
                }
            }
        }
    }
}

/* Function to add a message to the clientBuffer and set the POLLOUT event. */
void Server::messageBuffer(int clientFd, const std::string &message)
{
    clientBuffer[clientFd] += message;

    for (size_t i = 0; i < pollfds.size(); i++)
    {
        if (pollfds[i].fd == clientFd)
        {
            pollfds[i].events |= POLLOUT;
            break;
        }
    }
}

/* Function to run the server. It polls the pollfds vector and calls handleConnections
and handleClient when there is activity. It also calls sendMessage to send messages to
clients. */
void Server::run()
{
    std::cout << "Server running on port " << port << " with password " << password << std::endl;
    running = true;
    while (running)
    {
        int ret = poll(pollfds.data(), pollfds.size(), -1);
        if (ret == -1)
        {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }

        for (size_t i = 0; i < pollfds.size(); ++i)
        {
            if (pollfds[i].revents & POLLIN)
            {
                if (pollfds[i].fd == serverSocket)
                {
                    handleConnections();
                }
                else
                {
                    handleClient(pollfds[i].fd);
                }
            }
        }
    }
}

/* Function to handle a clean exit. It closes the server and exits with EXIT_SUCCESS. */
void Server::cleanExit()
{
    closeServer();
    exit(EXIT_SUCCESS);
}