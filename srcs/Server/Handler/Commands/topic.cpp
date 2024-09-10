#include "../Handler.hpp"

void Handler::topicCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	std::string channel;
	std::string trash;
	std::string topic;
	iss >> trash;
	if (!(iss >> channel))
	{
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return;
	}
	Channel *tempChannel = Server::getChannel(channel);
	if (tempChannel == NULL)
	{
		std::string error = ":server 403 " + channel + " :No Such Channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (Server::getChannel(channel)->isTopicOpOnly())
	{
		if (!Server::getChannel(channel)->isOperator(client->getNick()))
		{
			std::string error = ":server 482 " + client->getNick() + " " + channel + " :You're not channel operator\r\n";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;		
		}
	}
	if (iss>>topic && topic[0] == ':')
	{
		topic = message.substr(message.find(":") + 1, message.find("\r\n"));
		tempChannel->setTopic(topic);
		std::string topicMessage = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " TOPIC " + channel + " :" + topic + "\r\n"; 
		tempChannel->sendToAll(topicMessage);
	}
	else if (tempChannel->getTopic().empty())
	{
		std::string topicMessage = ":server 331 " + client->getNick() + ' ' + channel + " :No topic has been set\r\n";
		send(client->getSocket(), topicMessage.c_str(), topicMessage.size(), 0);
	}
	else{
		std::string topicMessage = ":server 332 " + client->getNick() + ' ' + channel + " :" + tempChannel->getTopic() + "\r\n";
		send(client->getSocket(), topicMessage.c_str(), topicMessage.size(), 0);
	}
}