#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include <cstring>
#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <algorithm>
#include <arpa/inet.h>
#include <sstream>

class Client;
class Channel;

class Server{
	protected:
		int serverSocket;
		static int port;
		static int epfd;
		static std::string password;
		struct epoll_event events[64];
		struct epoll_event event;
		static std::vector<Client *>clientPool;
		static std::vector<Channel *>channelPool;
		static std::string message;
	public:
		static bool isUserInUse(std::string name);
		static bool isNickInUse(std::string name);
		static bool checkChannelName(std::string name);
		static void addToChannelPool(Channel *channel);
		static void clientPoolErase(int clientSocket);
		static void userLeaving(const std::string& nick);
		static void sendMessageToChannel(const std::string& nick, const std::string& message);
		static void sendToAllClients(std::string message);
		static int getEpFD();
		static std::string extractChannelName(const std::string message);
		static std::string getPassword();
		static std::string getMessage(){return message;};
		static Client *findClientBySocket(int clientSocket);
		static Client *findClientByName(std::string name);
		static std::vector<Client *> getClientPool(){return clientPool;};
		static std::vector<Channel *> getChannelPool(){return channelPool;};
		static Channel* getChannel(std::string name);
		void makeOperator(const std::string& channel, const std::string& nick);
		void setPassword(char *p);
		void setPort(char *p);
		void processData(int i);
		void start();
		void lobby();
		static void ctrlChandler(int signum);
		int getServerSocket();
		int getEventFd(int i);
		std::vector<std::string> splitString(const std::string &str, const std::string &delimiter);
		epoll_event *getEvents();
} ;