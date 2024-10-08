#pragma once

#include "../Server/server.hpp"

class Client{
	private:
		bool authenticated;
		std::string nick;
		std::string username;
		std::string host;
		int clientSocket;
		std::vector<std::string> channels;
		std::string messageBuffer;
		std::string tempData;
	public:
		//MESSAGE RECEIVED
		void appendToMessageBuffer(const std::string& data) {
        	messageBuffer += data;
    	}
		void clearMessageBuffer() {
        	messageBuffer.clear();
    	}
		void setMessageBuffer(std::string msg);
		std::string getMessageBuffer();
		//EPOLLOUT HANDLE
		void appendData(const std::string& data);
		std::string& getData();
		void clearData();
		//IS AUTHENTICATED
		void setAuthentication(bool yesorno);
		bool getAuthentication();
		//CHANNEL HELP FUNC
		void leaveChannel(std::string channelName);
		bool isInChannel(std::string channelName);
		std::vector<std::string>& getChannels();
		//NETWORK FUNCS
		void setHost(char *ip);
		std::string getHost();
		void setSocket(int socket);
		int getSocket();
		//CLASS FUNCS
		Client();
		Client(const Client &client);
		void clientQUittingMsg();
		//NICK FUNCS
		void setNick(std::string _nick_);
		std::string getNick();
		//USERNAME FUNCS
		void setUsername(std::string _nick_);
		std::string getUsername();
} ;