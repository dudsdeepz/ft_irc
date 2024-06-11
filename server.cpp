#include "server.hpp"

int Server::getEpFD(){return epfd;}
epoll_event *Server::getEvents(){return events;}
int Server::getEventFd(int i){return events[i].data.fd;}
int Server::getServerSocket(){return serverSocket;}

void Server::start()
{
	Server::serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (Server::serverSocket == -1)
		throw std::runtime_error("Failed to create socket");
	
	sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(6667);
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

	char buffer[1024];
	std::string name;
	while (name.empty()) {
        recv(clientSocket, buffer, 1024, 0);
        std::string data(buffer);
        size_t nickPos = data.find("NICK ");
        if (nickPos != std::string::npos)
        {
            name = data.substr(nickPos + 5);
            size_t endPos = name.find_first_of("\r\n");
            if (endPos != std::string::npos) {
                name = name.substr(0, endPos);
            }
        }
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
            if ((*it)->getNick() == name) {
                name = "";
                break;
            }
        }
	}
	newClient->setNick(name);
	std::cout << newClient->getNick() << " has joined the server" << std::endl;
}

void Server::processData(int i)
{
	char buffer[1024];
    int clientSocket = events[i].data.fd;
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived <= 0) {
        close(clientSocket);
        epoll_ctl(epfd, EPOLL_CTL_DEL, clientSocket, NULL);
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
            if ((*it)->getSocket() == clientSocket) {
                std::cout << "Client disconnected: " << (*it)->getNick() << std::endl;
                delete *it;
                clientPool.erase(it);
                break;
            }
        }
    } else {
        std::string message(buffer);
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
            if ((*it)->getSocket() == clientSocket) {
                processCommands(clientSocket, message);
				message.clear();
                break;
            }
        }
	}
}

std::string Server::extractChannelName(const std::string& message) {
    size_t hashPos = message.find("#");
    if (hashPos == std::string::npos) {
        return "";
    }
    size_t spacePos = message.find("\r\n", hashPos);
    if (spacePos == std::string::npos) {
        return message.substr(hashPos + 1);
    } else {
        return message.substr(hashPos, spacePos - hashPos);
    }
}

void Server::processCommands(int clientSocket, std::string& message)
{
	std::string command = message.substr(0, message.find(" "));
	if (command == "JOIN"){
        std::string channelName = extractChannelName(message);
        std::string nick;
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); ++it) {
            if ((*it)->getSocket() == clientSocket) {
                nick = (*it)->getNick();
                (*it)->joinChannel(channelName);
                break;
            }
        }
        std::string joinNotification = ":" + nick + " JOIN :" + channelName + "\r\n";
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
            if ((*it)->isInChannel(channelName)) {
                send((*it)->getSocket(), joinNotification.c_str(), joinNotification.size(), 0);
            }
        }
		std::string namesList = ":MyServer 353 " + nick + " = " + channelName + " :";
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
            if ((*it)->isInChannel(channelName)) {
                namesList += (*it)->getNick() + " ";
            }
        }
        namesList += "\r\n";
        for (std::vector<Client*>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
            if ((*it)->isInChannel(channelName)) {
                send((*it)->getSocket(), namesList.c_str(), namesList.size(), 0);
            }
        }
	} else if (command.find("PRIVMSG") != std::string::npos) {
		size_t commandPos = message.find("PRIVMSG") + 7;
        size_t channelPos = message.find(" ", commandPos);
        size_t messagePos = message.find(":", channelPos);
		size_t endOfMessagePos = message.find("\r\n", messagePos);
        if (messagePos == std::string::npos) {
            return;
        }
        std::string channelName = message.substr(channelPos + 1, messagePos - channelPos - 2);
      	std::string msg = message.substr(messagePos + 1, endOfMessagePos - messagePos - 1);
		std::string nick;
		for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++)
		{
			if ((*it)->getSocket() == clientSocket)
				nick = (*it)->getNick();
		}
        sendMessageToChannel(nick, channelName, msg);
    }
} 

void Server::sendMessageToChannel(const std::string& nick, const std::string& channelName, const std::string& message)
{
	std::string user;
	std::string host;
	user = nick;
    for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
		if ((*it)->getNick() == nick)
			host = (*it)->getHost();
	}
    std::string fullMessage = ":" + nick + "!" + user + "@" + host + " PRIVMSG " + channelName + " :" + message + "\r\n";
    for (std::vector<Client *>::iterator it = clientPool.begin(); it != clientPool.end(); it++) {
        if ((*it)->isInChannel(channelName) && (*it)->getNick() != nick) {
            send((*it)->getSocket(), fullMessage.c_str(), fullMessage.size(), 0);
        }
    }
}