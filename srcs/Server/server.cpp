#include "server.hpp"
#include "./Handler/Handler.hpp"

epoll_event *Server::getEvents(){return events;}
std::vector<Client *> Server::clientPool;
std::vector<Channel *> Server::channelPool;
std::string Server::message; 
std::string Server::password;
std::string Server::getPassword(){return password;}
int Server::port;
int Server::epfd;
int Server::getEpFD(){return epfd;}
int Server::getEventFd(int i){return events[i].data.fd;}
int Server::getServerSocket(){return serverSocket;}

void Server::start()
{
	Server::serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (Server::serverSocket == -1)
		throw std::runtime_error("Failed to create socket");
	
	sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(Server::serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{
        throw std::runtime_error("Failed to bind socket");
	}
	if (listen(Server::serverSocket, SOMAXCONN) < 0)
        throw std::runtime_error("Failed to listen on socket");
	epfd = epoll_create(1);
	event.events = EPOLLIN;
	event.data.fd = serverSocket;
	epoll_ctl(epfd, EPOLL_CTL_ADD, serverSocket, &event);
}

void Server::lobby()
{
	struct sockaddr_in clientAddress;
	socklen_t clientLen = sizeof(clientAddress);
	int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
	if (clientSocket < 0)
		throw std::runtime_error("Failed to accept client");
	Client *newClient = new Client;
	newClient->setHost(inet_ntoa(clientAddress.sin_addr));
	newClient->setSocket(clientSocket);
	clientPool.push_back(newClient);
	event.data.fd = clientSocket;
	epoll_ctl(epfd, EPOLL_CTL_ADD, clientSocket, &event);
}

Channel* Server::getChannel(std::string name){
	for (std::vector<Channel *>::iterator it = channelPool.begin(); it != channelPool.end(); it++)
	{
		if ((*it)->getName() == name)
			return *it;
	}
	return NULL;
}

bool Server::checkChannelName(std::string name){
	for (std::vector<Channel *>::iterator it = channelPool.begin(); it != channelPool.end(); it++)
	{
		if ((*it)->getName() == name)
			return true;
	}
	return false;
 }

void Server::setPassword(char *p){
	password = p;	
}

void Server::setPort(char *p){
	if (strlen(p) > 4)
			throw std::runtime_error("Port too big!");
	for (int i = 0; p[i]; i++)
	{
		if (!isdigit(p[i]))
			throw std::runtime_error("Port needs to be numerical!");
	}
	port = atoi(p);
 }

Client* Server::findClientBySocket(int clientSocket)
{
    for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++)
    {
        if ((*it)->getSocket() == clientSocket)
            return *it;
    }
    return 0;
}

void Server::processData(int i)
{
	char buffer[2024];
    int bytesReceived = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesReceived <= 0)
	{
		if (bytesReceived == 0)
		{
			std::cout << "Lost connection with ClientSocket : " << events[i].data.fd << "\n" << std::endl;
			Handler::quitSignal(Server::findClientBySocket(events[i].data.fd));
		}
		else
			std::cout << "Error receiving data from recv\n" << std::endl;
	}
	else {
		std::vector<std::string> commands = splitString(buffer, "\r\n");
        for (std::vector<std::string>::iterator it = commands.begin(); it != commands.end(); it++) {
            std::string message = *it;
			std::cout << message << std::endl;
			Handler::processCommands(findClientBySocket(events[i].data.fd), message);
			memset(buffer, 0, sizeof(buffer));
		}
	}
}

std::string Server::extractChannelName(const std::string message) {
    
	std::string unknown;
	if (message.find("PRIVMSG") != std::string::npos)
		unknown = message.substr(message.find("PRIVMSG") + 8, message.find(':') - 1);
	else
		unknown = message.substr(message.find("JOIN") + 5, message.find(':') - 1);
	unknown.erase(std::remove(unknown.begin(), unknown.end(), ' '), unknown.end());
	unknown.erase(std::remove(unknown.begin(), unknown.end(), '\r'), unknown.end());
	unknown.erase(std::remove(unknown.begin(), unknown.end(), '\n'), unknown.end());
	return unknown.substr(0, unknown.find(":"));
}

void Server::sendMessageToChannel(const std::string& nick, const std::string& message)
{
	std::string channelName = extractChannelName(message);
	std::string user;
	std::string host;
    for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
		if ((*it)->getNick() == nick){
			user = (*it)->getUsername();
			host = (*it)->getHost();
		}
	}
    std::string fullMessage = ":" + nick + "!" + user + "@" + host + " " + message + "\r\n";
	if (channelName.find("#") != std::string::npos)
	{
		for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
			if ((*it)->isInChannel(channelName) && (*it)->getNick() != nick) {
				send((*it)->getSocket(), fullMessage.c_str(), fullMessage.size(), 0);
			}
		}
	}
	else {
			for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
				if ((*it)->getNick() == channelName) {
					send((*it)->getSocket(), fullMessage.c_str(), fullMessage.size(), 0);
				}
		}
	}
}

void Server::makeOperator(const std::string& channel, const std::string& nick) {
    std::string command = "MODE " + channel + " +o " + nick + "\r\n";
    for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
        if ((*it)->isInChannel(channel)) {
            send((*it)->getSocket(), command.c_str(), command.size(), 0);
        }
    }
}


void Server::userLeaving(const std::string& nick)
{
	std::string user;
	std::string host;
	user = nick;
    for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
		if ((*it)->getNick() == nick)
			host = (*it)->getHost();
	}
    std::string fullMessage = ":" + nick + "!" + user + "@" + host + " QUIT" + "Leaving" + "\r\n";
}


void Server::clientPoolErase(int clientSocket)
{
	for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
		if ((*it)->getSocket() == clientSocket) {
			clientPool.erase(it);
			return ;
		}
	}
}

bool Server::isNickInUse(std::string name)
{
    for (std::vector<Client*>::const_iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
        if ((*it)->getNick() == name) {
            return true;
        }
    }
    return false;
}

bool Server::isUserInUse(std::string name)
{
    for (std::vector<Client*>::const_iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
        if ((*it)->getUsername() == name) {
            return true;
        }
    }
    return false;
}



std::vector<std::string> Server::splitString(const std::string &str, const std::string &delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    tokens.push_back(str.substr(start));
    return tokens;
}

void Server::addToChannelPool(Channel *channel){
	channelPool.push_back(channel);
}

void Server::sendToAllClients(std::string message){
	for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++)
		send((*it)->getSocket(), message.c_str(), message.size(), 0);
}

Client *Server::findClientByName(std::string name){
	for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++)
	{
		if ((*it)->getNick() == name)
			return (*it);
	}
	return NULL;
}

void Server::ctrlChandler(int signum){
	(void)signum;
	for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++)
		Handler::quitSignal(*it);
	// for (std::vector<Channel *>::iterator it = channelPool.begin(); it != channelPool.end(); it++)
	// 	delete *it;
	close(epfd);
	exit(signum);
}