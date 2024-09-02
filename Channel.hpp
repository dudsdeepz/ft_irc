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
		std::vector<std::string >inviteList;
		size_t limit;
		bool limit_on;
		bool invite_only;
		bool password_mode;
		std::string password;
		std::string topic;
		bool topicOpOnly;
	public:
		bool Channel::isPasswordMode();
		void setPassword(bool mode, std::string pas);
		std::string getPassword();
		int getLimitNum();
		bool getLimitmod();
		void setLimit(int lim, bool mod);
		void changeTopicOpOnly(bool mod);
		bool isTopicOpOnly();
		bool isUserInInviteList(std::string nick);
		void switchInviteMode(bool mode);
		bool isInviteOnly();
		void removeFromInviteList(std::string nick);
		void addToInviteList(std::string nick);
		void sendToAll(std::string message);
		void removeFromList(std::string message);
		std::string getTopic();
		void setTopic(std::string msg);
		bool isLimitReached();
		void addToNamesList(std::string name);
		std::string retrieveNamesList(std::string clientName);
		std::vector<std::string> retrieveOpList();
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