#include "../Handler.hpp"

void Handler::whoCommand(Client *client)
{
	std::istringstream iss(client->getMessageBuffer());
	std::string buffer;
	std::string channel;
	iss >> buffer;
	if (iss >> channel)
	{
		Channel *tempChannel = Server::getChannel(channel);
		std::vector<std::string > tempNamesList = tempChannel->getNameslist();
		for (std::vector<std::string >::iterator it = tempNamesList.begin(); it != tempNamesList.end(); it++)
		{
			std::string status;
			if (channel != "*")
			{
				if (tempChannel->isOperator(*it))
					status = '@';
				else
					status = '+';
			}
			std::string username = Server::findClientByName(*it)->getUsername();
			std::string message = ":server 352 " + client->getNick() + " " + channel + " localhost ft_irc " + *it + " H" + status + " :1 " + username  + "\r\n";
			send(client->getSocket(), message.c_str(), message.size(), 0);
		}
		std::string message = ":server 315 " + client->getNick() + " " + channel + " :End of WHO list\r\n";
		send(client->getSocket(), message.c_str(), message.size(), 0);
	}
	else
	{
		std::vector<Channel *>tempPool = Server::getChannelPool();
		for (std::vector<Channel *>::iterator ita = tempPool.begin(); ita != tempPool.end(); ita++)
		{
			std::vector<std::string > tempNamesList = (*ita)->getNameslist();
			for (std::vector<std::string >::iterator it = tempNamesList.begin(); it != tempNamesList.end(); it++)
			{
				std::string status;
				if (channel != "*")
				{
					if ((*ita)->isOperator(*it))
						status = '@';
					else
						status = '+';
				}
				std::string message = ":server 352 " + client->getNick() + " " + (*ita)->getName() + " localhost ft_irc " + *it + " H" + status + " :1 " + Server::findClientByName(*it)->getUsername() + "\r\n";
				send(client->getSocket(), message.c_str(), message.size(), 0);
			}
		}
		std::string message = ":server 315 " + client->getNick() + " " + channel + " :End of WHO list\r\n";
		send(client->getSocket(), message.c_str(), message.size(), 0);
	}
}