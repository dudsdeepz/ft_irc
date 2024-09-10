#include "../Handler.hpp"

void Handler::kickCommand(Client *client)
{
	std::string channel;
	std::string temp;
	std::string nick;
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	iss >> temp;
	if (iss>>channel)
	{
		if (!(iss>>nick))
		{
			std::string error = ":server 461 * KICK :Not enough parameters\r\n";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;
		}
	}
	else {
		channel.clear();
		nick.clear();
	}
	Channel *channelTemp = Server::getChannel(channel);
	if (channelTemp == NULL)
	{
		std::string error = ":server 403 " + channel + " :No Such Channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (!channelTemp->isOperator(client->getNick()))
	{
		std::string error = ":server 482 " + nick + " " + channel + " :You're not channel operator\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;		
	}
	Client *tempClient = Server::findClientByName(nick);
	if (tempClient == NULL || !tempClient->isInChannel(channel))
	{
		std::string error = ":server 441 " + nick + " " + channel + " :They aren't on that channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	std::string kickNotify = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " KICK " + channel + " " + nick + " :" + client->getNick() + "\r\n";
	channelTemp->sendToAll(kickNotify);
	channelTemp->removeOnNamesList(nick);
	if (channelTemp->isOperator(nick))
		channelTemp->removeToOperators(nick);
	tempClient->leaveChannel(channel);
}