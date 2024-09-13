#pragma once

#include <map>
#include "../../Client/Client.hpp"
#include "../../Channel/Channel.hpp"

class Client;
class Channel;

typedef void (*commandHandler)(Client* client);

class Handler {
	private:
		static std::map<std::string, commandHandler> handler;
		static std::vector<Channel *>channelsPool;	
	public:
		static void channelsReplace(std::string oldname, std::string name, Client *client);
		static void whoCommand(Client *client);
		static void passwordMode(bool toggle, Channel *channel, Client *client, char toggleC, char mod, std::string password);
		static bool isAllDigit(std::string msg);
		static void modeLimit(bool toggle, Channel *channel, Client *client, char toggleC, char mode, std::string limit);
		static void modeTopic(bool toggle, Channel *channel, Client *client, char toggleC, char mode);
		static void modeOperator(bool toogle, Channel *channel, std::string nick, Client *client, char toggleC, char mode);
		static void modeInviteOnly(bool toogle, Channel *channel, Client *client, char toggleC, char mode);
		static void modeCommand(Client *client);
		static void partCommand(Client *client);
		static void invCommand(Client *client);
		static void kickCommand(Client *client);
		static void topicCommand(Client *client);
		static void privmsgCommand(Client *client);
		static void nickCommand(Client *client);
		static void usernameCommand(Client *client);
		static void registerCommands();
		static void capCommand(Client *client);
		static void processCommands(Client *client, std::string& message);
		static void joinCommand(Client* client);
		static void authCommand(Client* client);
		static void quitSignal(Client* client);
};
