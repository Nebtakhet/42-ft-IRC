#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <cstring>
# include <cstdlib>
# include <iostream>
# include <vector>
# include <map>
# include <poll.h>
# include <unistd.h>
# include <sstream>
# include <string>
# include <cerrno>
# include <ctime>
# include <algorithm>
# include <set>

class Client
{
    private:
        int				_clientFd;
        std::string		_read_buffer;
        std::string		_send_buffer;
        std::string		_nickname;
        std::string		_old_nickname;
        std::string		_username;
        std::string		_realname;
        std::string		_mode;
        std::vector<std::string> _capabilities;
        bool            _authenticated;
        bool            _operator;
        bool            _capNegotiation; // New flag for CAP negotiation state
		bool			_welcomeSent; // Flag to check if welcome message has been sent
		
        std::set<std::string> joinedChannels;

    
    public:
        Client(int clientFd);
        ~Client();
        
        int				getClientFd()const;
        void			setNickname(std::string const &nickname);
        std::string&	getReadBuffer();
        void			setReadBuffer(std::string const &buf);
        std::string&	getSendBuffer();
        void			setSendBuffer(std::string const &buf);

        const std::string& getNickname() const; 
        void			setOldNickname(std::string const &nickname);
        std::string&	getOldNickname();
        void			setUsername(std::string const &username);
        std::string		getUsername()const;
        void			setRealname(std::string const &realname);
        std::string		getRealname()const;

        bool            isOperator() {return _operator;}
        std::string&	getMode();
        void			addMode(const std::string &mode);
        void			removeMode(const std::string &mode);

        void addCapability(const std::string &capability);
        bool hasCapability(const std::string &capability) const;
        void clearCapabilities();

        void setAuthenticated(bool authenticated); 
        bool isAuthenticated() const;

        void setCapNegotiation(bool capNegotiation);
        bool isCapNegotiating() const; 

		void setWelcomeSent(bool welcomeSent) { _welcomeSent = welcomeSent; }
		bool isWelcomeSent() const { return _welcomeSent; }

        void joinChannel(const std::string &channelName) { joinedChannels.insert(channelName); }
        void leaveChannel(const std::string &channelName) { joinedChannels.erase(channelName); }
        const std::set<std::string> &getJoinedChannels() const { return joinedChannels; }
};

#endif