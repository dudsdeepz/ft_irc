#include "../Handler.hpp"

void Handler::partCommand(Client *client)
{
	std::string channel;
	std::string temp;
	std::string reason;
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	iss >> temp;
	if (!(iss>>channel) || !(iss>>reason))
	{
		std::string error = ":server 461 * PART :Not enough parameters\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	Channel *channelTemp = Server::getChannel(channel);
	if (channelTemp == NULL)
	{
		std::string error = ":server 403 " + channel + " :No Such Channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	std::string partNotify = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " PART " + channel + " " + reason + "\r\n";
	send(client->getSocket(), partNotify.c_str(), partNotify.size(), 0);
	channelTemp->sendToAll(partNotify);
	channelTemp->removeOnNamesList(client->getNick());
	if (channelTemp->isOperator(client->getNick()))
		channelTemp->removeToOperators(client->getNick());
	client->leaveChannel(channel);
}