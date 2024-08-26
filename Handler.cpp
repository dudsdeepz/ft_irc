#include "Handler.hpp"
#include "server.hpp"

std::map<std::string, commandHandler> Handler::handler;

void Handler::registerCommands()
{
	handler["JOIN"] = joinCommand;
	handler["AUTH"] = authCommand;
	handler["NICK"] = nickCommand;
	handler["USER"] = usernameCommand;
	handler["CAP"] = capCommand;
	handler["PRIVMSG"] = privmsgCommand;
	handler["TOPIC"] = topicCommand;
	handler["KICK"] = kickCommand;
	handler["QUIT"] = quitSignal;
	// handler["MODE"] = modeCommand;

}

void Handler::topicCommand(Client *client)
{

	std::string message = client->getMessageBuffer();
	if (message.find("#") == std::string::npos)
	{
		sendLogMessage(client, "No such channel");
		return ;
	}
	std::string channel = message.substr(message.find("#"), message.find("\r\n"));
	std::string topicMessage = ":MyServer 331 " + client->getNick() + ' ' + channel + " :" + Server::getChannel(channel)->getTopic() + "\r\n";
	std::cout << topicMessage << std::endl;
	if (send(client->getSocket(), topicMessage.c_str(), topicMessage.size(), 0) < 0)
		std::cout << "Error sending topic\r\n" << std::endl;
}

void Handler::privmsgCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	Server::sendMessageToChannel(client->getNick(), message);
}

void Handler::capCommand(Client *client)
{
	(void)client;
}

void Handler::nickCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	std::string name = message.substr(message.find("NICK") + 5, message.find("\n"));
	name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
	if (Server::isNickInUse(name))
	{
		std::string nickInUse = ":server 433 * " + name + " :Nickname is already in use.\r\n";
		send(client->getSocket(), nickInUse.c_str(), nickInUse.size(), 0);
		client->setNick(name);
		return ;
	}
	if (isdigit(name[0]) || name[0] == '#' || name[0] == ':' ||( (int)name.find(" ") && (int)name.find(" ") != -1))
	{
		std::string nickInUse = ":server 432 * " + name +":Erroneus nickname\r\n";
		send(client->getSocket(), nickInUse.c_str(), nickInUse.size(), 0);
		return ;
	}
	if (client->getChannels().size() > 0)
	{
		std::string notifyNickChange = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " NICK :" + name + "\r\n";
		Server::sendToAllClients(notifyNickChange);
	}
	std::string oldname = client->getNick();
	client->setNick(name);
	std::string nickChange = ":" + oldname + " NICK " + name + "\r\n";
	send(client->getSocket(), nickChange.c_str(), nickChange.size(), 0);
}

void Handler::usernameCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	std::string name = message.substr(message.find("USER") + 4, message.find("0"));
	name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
	client->setUsername(name.substr(0, name.find("0")));
}

void Handler::processCommands(Client *client, std::string& message)
{
	std::string command = message.substr(0, message.find(" "));
	std::map<std::string, commandHandler>::iterator handleIt = handler.find(command);
	if (!message.size())
		return ; 
	client->setMessageBuffer(message);
	if (handleIt != handler.end())
		(*handleIt).second(client);
}

void Handler::joinCommand(Client* client)
{
	if (!client->getAuthentication())
	{
		sendLogMessage(client, "You need to authenticate yourself first !");
		return ;
	}
	std::vector<Client *>clientPool = Server::getClientPool();
	std::string channelName = Server::extractChannelName(client->getMessageBuffer());
	if (client->isInChannel(channelName))
	{
		sendLogMessage(client, "Already in channel");
		return;
	}
	if (channelName.empty())
	{
		sendLogMessage(client, "Please insert a valid channel name !");
		return ;
	}
	if (!Server::checkChannelName(channelName))
	{
		Channel* newChannel = new Channel(channelName, client->getNick());
		newChannel->addToOperators(client->getNick());
		Server::addToChannelPool(newChannel);
	}
	client->getChannels().push_back(channelName);
	Channel* tempChannel = Server::getChannel(channelName);
	tempChannel->addToNamesList(client->getNick());
	std::string joinNotification = ":" + client->getNick() + " JOIN :" + channelName + "\r\n";
	std::string channelNamesList = tempChannel->retrieveNamesList(client->getNick());
	tempChannel->sendToAll(joinNotification);
	tempChannel->sendToAll(channelNamesList);
	
}

void Handler::authCommand(Client* client)
{
	if (client->getNick().empty() || client->getUsername().empty())
	{
		std::string nickInUse = ":server 432 * " + client->getNick() +":Erroneus nickname\r\n";
		send(client->getSocket(), nickInUse.c_str(), nickInUse.size(), 0);
		return ;
	}
	std::string message = client->getMessageBuffer();
	if (message.find(Server::getPassword()) != std::string::npos)
	{
		client->setAuthentication(true);
		sendLogMessage(client, ("Welcome\t" + client->getNick() + " !"));
	}
	else
		sendLogMessage(client, "Invalid password !");
}

void Handler::quitSignal(Client* client)
{
	client->clientQUittingMsg();
	epoll_ctl(Server::getEpFD(), EPOLL_CTL_DEL, client->getSocket(), NULL);
	close(client->getSocket());
	std::cout << "Client disconnected: " << (client)->getNick() << std::endl;
	Server::clientPoolErase(client->getSocket());
	delete client;
}

void Handler::sendLogMessage(Client* client, std::string message){
	message += "\r\n";
	send(client->getSocket(), message.c_str(), message.length(), 0);
}

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
			std::string error = ":server 461 * KICK :Not enough parameters";
			send(client->getSocket(), error.c_str(), error.size(), 0);
			return ;
		}
	}
	else {
		channel.clear();
		nick.clear();
	}
	std::cout << "nick:" << nick << std::endl;
	std::cout << "channel:" << channel << std::endl;
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