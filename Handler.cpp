#include "Handler.hpp"
#include "server.hpp"

std::map<std::string, commandHandler> Handler::handler;

void Handler::registerCommands()
{
	handler["JOIN"] = joinCommand;
	handler["PASS"] = authCommand;
	handler["NICK"] = nickCommand;
	handler["USER"] = usernameCommand;
	handler["CAP"] = capCommand;
	handler["PRIVMSG"] = privmsgCommand;
	handler["TOPIC"] = topicCommand;
	handler["KICK"] = kickCommand;
	handler["QUIT"] = quitSignal;
	handler["INVITE"] = invCommand;
	handler["PART"] = partCommand;
	handler["MODE"] = modeCommand;

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
	std::string name = message.substr(message.find("NICK") + 5, message.find("\r\n"));
	name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
	name.erase(std::remove(name.begin(), name.end(), '\r'), name.end());
	name.erase(std::remove(name.begin(), name.end(), '\n'), name.end());
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
	std::istringstream iss(message);
	std::vector<std::string >buffer;
	std::string buffers;
	while (iss >> buffers)
		buffer.push_back(buffers);
	if (buffer.size() < 5)
	{
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return;
	}
	if (buffer.size() > 5)
	{
		std::string error = ":server 461 * USER :Too many parameters\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return;
	}
	if (Server::isUserInUse(buffer[1]))
	{
		std::string error = ":server 462 " + client->getNick() + " :You may not register\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (buffer[2] != "0" || buffer[3] != "*" || buffer[4][0] != ':')
	{
		std::string error = ":server 421 USER : USAGE: USER <username> 0 * :<realname>\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	std::string name = message.substr(message.find("USER") + 4, message.find("0"));
	name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
	client->setUsername(name.substr(0, name.find("0")));
}

void Handler::processCommands(Client *client, std::string& message)
{
	if (!message.size())
		return ; 
	std::string command = message.substr(0, message.find(" "));
	std::map<std::string, commandHandler>::iterator handleIt = handler.find(command);
	client->setMessageBuffer(message);
	if (handleIt != handler.end())
	{
		if (client->getNick().empty() || client->getUsername().empty())
		{
			if (command != "NICK" && command != "USER" && command != "CAP")
			{
				std::string error = ":server 451 : You have not registered\r\n";
				send(client->getSocket(), error.c_str(), error.size(), 0);
				return ;
			}
		}
		(*handleIt).second(client);
	}
	else{
		std::string unknownCommand = ":server 421 " + command + " :Unknown command\r\n";
		send(client->getSocket(), unknownCommand.c_str(), unknownCommand.size(), 0);
	}
}

void Handler::joinCommand(Client* client)
{
	if (!client->getAuthentication())
	{
		std::string error = ":server 451 : You have not authenticated\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	std::vector<Client *>clientPool = Server::getClientPool();
	std::string channelName = Server::extractChannelName(client->getMessageBuffer());
	if (channelName.find("#") == std::string::npos)
	{
		sendLogMessage(client , "Invalid channel name");
		return ;
	}
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
	Channel* tempChannel = Server::getChannel(channelName);
	if (tempChannel->isUserInInviteList(client->getNick()))
		tempChannel->removeFromInviteList(client->getNick());
	else if (tempChannel->isInviteOnly())
	{
		std::string error = ":server 473 " + client->getNick() + " " + channelName + " :Cannot join channel (+i)\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	client->getChannels().push_back(channelName);
	tempChannel->addToNamesList(client->getNick());
	std::string joinNotification = ":" + client->getNick() + " JOIN :" + channelName + "\r\n";
	std::string channelNamesList = tempChannel->retrieveNamesList(client->getNick());
	tempChannel->sendToAll(joinNotification);
	tempChannel->sendToAll(channelNamesList);
	
}

void Handler::authCommand(Client* client)
{
	std::string message = client->getMessageBuffer();
	if (client->getAuthentication())
	{
		std::string error = ":server 462 " + client->getNick() + " :Already authenticated\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (message.find(Server::getPassword()) != std::string::npos)
	{
		client->setAuthentication(true);
		sendLogMessage(client, ("Welcome\t" + client->getNick() + " !"));
	}
	else
	{
		std::string error = ":server 464 " + client->getNick() + " :Password incorrect\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
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

void Handler::modeCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	std::string tempTrash;
	std::string channel;
	std::vector<std::string > arguements;
	bool toggle;

	iss >> tempTrash;
	iss >> channel;
	tempTrash.clear();
	if (!Server::getChannel(channel)->isOperator(client->getNick()))
	{
		std::string error = ":server 482 " + client->getNick() + " " + channel + " :You're not channel operator\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	while (iss >> tempTrash)
		arguements.push_back(tempTrash);
	for (std::vector<std::string>::iterator it = arguements.begin(); it != arguements.end(); it++)
	{
		std::string message = *it;
		for (int i = 0; message[i]; i++)
		{
			if (message[i] == '+')
				toggle = true;
			else if (message[i] == '-')
				toggle = false;
			else if (message[i] == 'i' || message[i] == 't' || message[i] == 'k' || message[i] == 'o')
			{
				std::string messageC;
				char toggleC;
				if (toggle == true)
					toggleC = '+';
				else
					toggleC = '-';
				switch(message[i])
				{
					case 'i':
						modeInviteOnly(toggle, Server::getChannel(channel));
						messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel + " " + toggleC + message[i] + "\r\n";
						send(client->getSocket(), messageC.c_str(), messageC.size(), 0);
					break;
					case 't':
					break;
					case 'k':
					break;
					case 'o':
						modeOperator(toggle, Server::getChannel(channel), *(it + 1));
						messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel + " " + toggleC + message[i] + " " + *(it + 1) +"\r\n";
						Server::getChannel(channel)->sendToAll(messageC);
						Server::getChannel(channel)->sendToAll(Server::getChannel(channel)->retrieveNamesList(*(it + 1)));
					break;
					default:
							std::string error = ":server 501  :Unknown MODE flag\r\n";
							send(client->getSocket(), error.c_str(), error.size(), 0);
					break;
				}
			}
		}
	}

}

// void Handler::modeTopic(std::string topic, bool toggle)
// {

// }

void Handler::modeInviteOnly(bool toggle, Channel *channel)
{
	if (toggle == channel->isInviteOnly())
		return;
	else
		channel->switchInviteMode(toggle);
}

void Handler::modeOperator(bool toggle, Channel *channel, std::string nick)
{
	if (!channel->isUserOnThisChannel(nick))
		return ;
	if (toggle == true && !channel->isOperator(nick))
		channel->addToOperators(nick);
	else if (toggle == false && channel->isOperator(nick))
		channel->removeToOperators(nick);
}