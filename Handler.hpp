#pragma once

#include <map>
#include "Client.hpp"
#include "Channel.hpp"

class Client;
class Channel;

typedef void (*commandHandler)(Client* client);

class Handler {
	private:
		static std::map<std::string, commandHandler> handler;
		static std::vector<Channel *>channelsPool;	
	public:
		static void kickCommand(Client *client);
		static void topicCommand(Client *client);
		static void privmsgCommand(Client *client);
		static void nickCommand(Client *client);
		static void usernameCommand(Client *client);
		static void sendLogMessage(Client* client, std::string message);
		static void registerCommands();
		static void capCommand(Client *client);
		static void processCommands(Client *client, std::string& message);
		static void joinCommand(Client* client);
		static void authCommand(Client* client);
		static void quitSignal(Client* client);
};
