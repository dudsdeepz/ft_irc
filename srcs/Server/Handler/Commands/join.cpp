#include "../Handler.hpp"

void Handler::joinCommand(Client* client)
{
	if (!client->getAuthentication())
	{
		std::string error = ":server 451 : You have not authenticated\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	std::vector<Client *>clientPool = Server::getClientPool();
	std::istringstream iss(client->getMessageBuffer());
	std::string buffer;
	std::string channelName;
	iss >> buffer;
	iss >> channelName;
	if (channelName[0] != '#')
	{
		std::string error = ":server 403 " + client->getNick() + " " + channelName + " :Invalid channel name\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (client->isInChannel(channelName))
		return;
	if (channelName.empty())
	{
		std::string message = ":server 461 JOIN :Not enough parameters\r\n";
		send(client->getSocket(), message.c_str(), message.size(), 0);
		return ;
	}
	if (!Server::checkChannelName(channelName))
	{
		Channel* newChannel = new Channel(channelName, client->getNick());
		Server::addToChannelPool(newChannel);
	}
	Channel* tempChannel = Server::getChannel(channelName);
	if (tempChannel->isPasswordMode())
	{
		std::istringstream iss (client->getMessageBuffer());
		std::string buffer;
		std::vector<std::string > args;
		while (iss >> buffer)
			args.push_back(buffer);
		if (args.size() != 3)
		{
			std::string error = ":server 475 " + client->getNick() + " " + channelName + " :Cannot join channel (+k)\r\n";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;
		}
		if (args[2] != tempChannel->getPassword())
		{
			std::cout << args[2] <<  "|" << std::endl;
			std::cout << tempChannel->getPassword() << "|" << std::endl;
			std::string error = ":server 475 " + client->getNick() + " " + channelName + " :Invalid Password (+k)\r\n";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;
		}
	}	
	if (tempChannel->getLimitmod())
	{
		if (tempChannel->isLimitReached()){
			std::string error = ":server 471 " + client->getNick() + " " + channelName + " :Channel limit reached (+l)\r\n";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;
		}
	}
	if (tempChannel->isUserInInviteList(client->getNick()))
		tempChannel->removeFromInviteList(client->getNick());
	else if (tempChannel->isInviteOnly())
	{
		std::string error = ":server 473 " + client->getNick() + " " + channelName + " :Cannot join channel (+i)\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (tempChannel->getNameslist().size() == 0)
		tempChannel->addToOperators(client->getNick());
	client->getChannels().push_back(channelName);
	tempChannel->addToNamesList(client->getNick());
	std::string joinNotification = ":" + client->getNick() + " JOIN :" + channelName + "\r\n";
	std::string channelNamesList = tempChannel->retrieveNamesList(client->getNick());
	tempChannel->sendToAll(joinNotification);
	tempChannel->sendToAll(channelNamesList);
	
}