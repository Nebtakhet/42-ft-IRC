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
    
    public:
        Client(int clientFd);
        ~Client();
        
        int				getClientFd()const;
        void			setNickname(std::string const &nickname);
        std::string&	getReadBuffer();
        void			setReadBuffer(std::string const &buf);
        std::string&	getSendBuffer();
        void			setSendBuffer(std::string const &buf);

        std::string&	getNickname();
        void			setOldNickname(std::string const &nickname);
        std::string&	getOldNickname();
        void			setUsername(std::string const &username);
        std::string		getUsername()const;
        void			setRealname(std::string const &realname);
        std::string		getRealname()const;

        std::string&	getMode();
        void			addMode(std::string const mode);
        void			removeMode(std::string const mode);
};

#endif