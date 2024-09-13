#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <limits.h>
#include "../Server/server.hpp"

class Channel{
	private:
		//CHANNEL DATA
		std::string name_;
		std::vector<std::string >operators;
		std::vector<std::string >namesList;
		std::vector<std::string >inviteList;
		std::string password;
		std::string topic;
		//MODES
		size_t limit;
		bool limit_on;
		bool invite_only;
		bool password_mode;
		bool topicOpOnly;
	public:
		//K MODE
		bool isPasswordMode();
		void setPassword(bool mode, std::string pas);
		std::string getPassword();
		//L MODE
		int getLimitNum();
		bool getLimitmod();
		void setLimit(int lim, bool mod);
		bool isLimitReached();
		//T MODE
		void changeTopicOpOnly(bool mod);
		bool isTopicOpOnly();
		std::string getTopic();
		void setTopic(std::string msg);
		//I MODE
		bool isUserInInviteList(std::string nick);
		void switchInviteMode(bool mode);
		bool isInviteOnly();
		void removeFromInviteList(std::string nick);
		void addToInviteList(std::string nick);
		//HELP FUNTIONS
		void sendToAll(std::string message);
		void setName(char *name);
		std::string getName();
		bool isUserOnThisChannel(std::string name);
		Channel(std::string name, std::string op);
		~Channel();
		//NAMESLIST
		void removeFromList(std::string message);
		void addToNamesList(std::string name);
		std::string retrieveNamesList(std::string clientName);
		std::vector<std::string> getNameslist();
		void removeOnNamesList(std::string name);
		void replaceOnNamesList(std::string name, std::string oldname);
		//OP LIST
		std::vector<std::string> retrieveOpList();
		void addToOperators(std::string name);
		void removeToOperators(std::string name);
		bool isOperator(std::string name);
		void replaceOnOpList(std::string name, std::string oldname);
}	;