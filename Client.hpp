#pragma once

#include "server.hpp"

class Client{
	private:
		bool authenticated;
		std::string nick;
		std::string username;
		std::string host;
		int clientSocket;
		std::vector<std::string> channels;
		std::string messageBuffer;
	public:
		void setMessageBuffer(std::string msg);
		std::string getMessageBuffer();
		void setAuthentication(bool yesorno);
		bool getAuthentication();
		bool isInChannel(std::string channelName);
		std::vector<std::string>& getChannels();
		void setHost(char *ip);
		std::string getHost();
		void setSocket(int socket);
		int getSocket();
		Client();
		Client(const Client &client);
		void setNick(std::string _nick_);
		std::string getNick();
		void setUsername(std::string _nick_);
		std::string getUsername();
		void leaveChannel(std::string channelName);
		void clientQUittingMsg();
		~Client();
} ;