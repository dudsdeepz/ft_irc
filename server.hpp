#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <csignal>
#include <cstring>
#include "Client.hpp"
#include <sys/epoll.h>
#include <fcntl.h>
#include <algorithm>
#include <arpa/inet.h>

class Client;

class Server{
	private:
		int serverSocket;
		struct epoll_event events[64];
		int epfd;
		struct epoll_event event;
		std::vector<Client *>clientPool;
	public:
		void sendMessageToChannel(const std::string& nick, const std::string& channelName, const std::string& message);
		std::string extractChannelName(const std::string& message);
		void processCommands(int clientSocket, std::string& message);
		int getServerSocket();
		int getEventFd(int i);
		void processData(int i);
		std::vector<Client *> clients;
		int getEpFD();
		epoll_event *getEvents();
		void start();
		void lobby();
} ;