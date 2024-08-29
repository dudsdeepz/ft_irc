#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include <cstring>
#include "Client.hpp"
#include "Channel.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <algorithm>
#include <arpa/inet.h>
#include <sstream>

class Client;
class Channel;

class Server{
	protected:
		static int port;
		static std::string password;
		int serverSocket;
		struct epoll_event events[64];
		static int epfd;
		struct epoll_event event;
		static std::vector<Client *>clientPool;
		static std::vector<Channel *>channelPool;
		static std::string message;
	public:
		static bool isUserInUse(std::string name);
		static bool isNickInUse(std::string name);
		static void clientPoolErase(int clientSocket);
		static std::string getPassword();
		void setPassword(char *p);
		void setPort(char *p);
		static void userLeaving(const std::string& nick);
		static std::string getMessage(){return message;};
		static Client *findClientBySocket(int clientSocket);
		static Client *findClientByName(std::string name);
		static std::vector<Client *> getClientPool(){return clientPool;};
		void makeOperator(const std::string& channel, const std::string& nick);
		static void sendMessageToChannel(const std::string& nick, const std::string& message);
		static std::string extractChannelName(const std::string message);
		int getServerSocket();
		int getEventFd(int i);
		static std::vector<Channel *> getChannelPool();
		static bool checkChannelName(std::string name);
		static Channel* getChannel(std::string name);
		static void addToChannelPool(Channel *channel);
		void processData(int i);
		std::vector<Client *> clients;
		static int getEpFD();
		epoll_event *getEvents();
		std::vector<std::string> splitString(const std::string &str, const std::string &delimiter);
		static void sendToAllClients(std::string message);
		void start();
		void lobby();
} ;