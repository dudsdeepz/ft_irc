#pragma once

#include "server.hpp"

class Client{
	private:
		std::string nick;
		std::string username;
		std::string host;
		int clientSocket;
		std::vector<std::string> channels;
	public:
		bool isInChannel(std::string channelName);
		void joinChannel(const std::string& channelName);
		const std::vector<std::string>& getChannels();
		void setHost(char *ip);
		std::string getHost();
		void setSocket(int socket);
		int getSocket();
		Client();
		Client(const Client &client);
		void setNick(std::string _nick_);
		std::string getNick();
		~Client();
} ;