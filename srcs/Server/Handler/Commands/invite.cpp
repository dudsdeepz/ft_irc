#include "../Handler.hpp"

void Handler::invCommand(Client *client)
{
	std::string channel;
	std::string temp;
	std::string nick;
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	iss >> temp;
	if (iss>>nick)
	{
		if (!(iss>>channel))
		{
			std::string error = ":server 461 * KICK :Not enough parameters";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;
		}
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
		std::string error = ":server 482 " + client->getNick() + " " + channel + " :You're not channel operator\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (Server::findClientByName(nick) == NULL)
	{
		std::string error = ":server 401 " + client->getNick() + " " + nick + " :No such nick\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (!client->isInChannel(channel))
	{
		std::string error = ":server 442 " + client->getNick() + " " + channel + " :You're not on that channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (Server::findClientByName(nick)->isInChannel(channel))
	{
		std::string error = ":server 443 " + client->getNick() + " " + nick + " " + channel + " :is already on channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	std::string invNotify = ":server 341 " + client->getNick() + " " + nick + " " + channel + "\r\n";
	send(client->getSocket(), invNotify.c_str(), invNotify.size(), 0);
	std::string invNick = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " INVITE " + nick + " " + channel + "\r\n";
	send(Server::findClientByName(nick)->getSocket(), invNick.c_str(), invNick.size(), 0);
	channelTemp->addToInviteList(nick);
}
