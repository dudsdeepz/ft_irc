#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <limits.h>
#include "server.hpp"

class Server;

class Channel{
	private:
		std::string name_;
		std::vector<std::string >operators;
		std::vector<std::string >namesList;
		int limit;
		bool invite_only;
		std::string password;
		std::string topic;
	public:
		void sendToAll(std::string message);
		void removeFromList(std::string message);
		std::string getTopic();
		void setTopic(std::string msg);
		bool isLimitReached();
		void addToNamesList(std::string name);
		std::string retrieveNamesList(std::string clientName);
		Channel(std::string name, std::string op);
		std::vector<std::string> getNameslist();
		void setName(char *name);
		void addToOperators(std::string name);
		void removeToOperators(std::string name);
		bool isOperator(std::string name);
		void removeOnNamesList(std::string name);
		bool isUserOnThisChannel(std::string name);
		std::string getName();
}	;