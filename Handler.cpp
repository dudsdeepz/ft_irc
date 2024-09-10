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
	handler["WHO"] = whoCommand;
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

void Handler::privmsgCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	Server::sendMessageToChannel(client->getNick(), message);
}

void Handler::capCommand(Client *client)
{
	std::string message = "this does nothing:\r\n";
	send(client->getSocket(), message.c_str(), message.size(), 0);
}

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
			std::string message = ":server 352 " + client->getNick() + " " + channel + " localhost ft_irc " + *it + " H" + status + " :1 " + Server::findClientByName(*it)->getUsername() + "\r\n";
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

void Handler::authCommand(Client* client)
{
	std::string passwordCheck;
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	iss >> passwordCheck;
	passwordCheck.clear();
	iss >> passwordCheck;
	if (client->getAuthentication())
	{
		std::string error = ":server 462 " + client->getNick() + " :Already authenticated\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	if (passwordCheck == Server::getPassword())
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
	if (!Server::checkChannelName(channel))
	{
		std::string error = ":server 403 " + channel + " :No Such Channel\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	while (iss >> tempTrash)
		arguements.push_back(tempTrash);
	if (!arguements.size())
		return ;
	if (!Server::getChannel(channel)->isOperator(client->getNick()))
	{
		std::string error = ":server 482 " + client->getNick() + " " + channel + " :You're not channel operator\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	bool checkToggle = false;
	for (std::vector<std::string>::iterator it = arguements.begin(); it != arguements.end(); it++)
	{
		std::string message = *it;
		for (int i = 0; message[i]; i++)
		{
			if (message[i] == '+')
			{
				toggle = true;
				checkToggle = true;
			}
			else if (message[i] == '-')
			{
				toggle = false;
				checkToggle = true;
			}
			else if ((message[i] == 'i' || message[i] == 't' || message[i] == 'k' || message[i] == 'o' || message[i] == 'l') && checkToggle)
			{
				std::string messageC;
				char toggleC;
				if (toggle == true)
					toggleC = '+';
				else
					toggleC = '-';
				if (message[i] == 'k' || message[i] == 'o' || (message[i] == 'l' && toggle))
                {
                    if (it + 1 == arguements.end())
                    {
                        std::string error = ":server 461 " + client->getNick() + " " + channel + " :Not enough parameters\r\n";
                        send(client->getSocket(), error.c_str(), error.size(), 0);
                        return;
                    }
                }
				switch(message[i])
				{
					case 'i':
						modeInviteOnly(toggle, Server::getChannel(channel), client, toggleC, message[i]);
					break;
					case 't':
						modeTopic(toggle, Server::getChannel(channel), client, toggleC, message[i]);
					break;
					case 'k':
						passwordMode(toggle, Server::getChannel(channel), client, toggleC, message[i], *(it + 1));
						arguements.erase(std::remove(arguements.begin(), arguements.end(), *(it + 1)), arguements.end());
					break;
					case 'o':
						modeOperator(toggle, Server::getChannel(channel), *(it + 1), client, toggleC, message[i]);
						arguements.erase(std::remove(arguements.begin(), arguements.end(), *(it + 1)), arguements.end());
					break;
					case 'l':
						modeLimit(toggle, Server::getChannel(channel), client, toggleC, message[i], *(it + 1));
						arguements.erase(std::remove(arguements.begin(), arguements.end(), *(it + 1)), arguements.end());
					break;
				}
			}
			else{
				std::string error = ":server 501  :Unknown MODE flag\r\n";
				send(client->getSocket(), error.c_str(), error.size(), 0);
				continue ;
			}
		}
	}
}

void Handler::modeInviteOnly(bool toggle, Channel *channel, Client *client, char toggleC, char mode)
{
	if (toggle == channel->isInviteOnly())
		return;
	else
		channel->switchInviteMode(toggle);
	std::string messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel->getName() + " " + toggleC + mode + "\r\n";
	channel->sendToAll(messageC);
}

void Handler::modeOperator(bool toggle, Channel *channel, std::string nick, Client *client, char toggleC, char mode)
{
	if (nick.empty())
		return ;
	if (!channel->isUserOnThisChannel(nick))
		return ;
	if (channel->isOperator(nick) && toggle)
		return ;
	if (toggle == true && !channel->isOperator(nick))
		channel->addToOperators(nick);
	else if (toggle == false && channel->isOperator(nick))
		channel->removeToOperators(nick);
	std::string messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel->getName() + " " + toggleC + mode + " " + nick +"\r\n";
	channel->sendToAll(messageC);
	channel->sendToAll(channel->retrieveNamesList(nick));
}

void Handler::modeTopic(bool toggle, Channel *channel, Client *client, char toggleC, char mode)
{
	if (toggle == channel->isTopicOpOnly())
		return;
	else
		channel->changeTopicOpOnly(toggle);
	std::string messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel->getName() + " " + toggleC + mode + "\r\n";
	channel->sendToAll(messageC);
}

void Handler::modeLimit(bool toggle, Channel *channel, Client *client, char toggleC, char mode, std::string limit)
{
	if (!isAllDigit(limit))
		return;
	else if (channel->getLimitNum() == atoi(limit.c_str()) && toggle)
		return ;
	else if (!toggle)
		channel->setLimit(0, toggle);
	else
		channel->setLimit(atoi(limit.c_str()), toggle);
	std::string messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel->getName() + " " + toggleC + mode + " " + limit + "\r\n";
	channel->sendToAll(messageC);
}

bool Handler::isAllDigit(std::string msg)
{
	for (int i = 0; msg[i]; i++)
	{
		if (!isdigit(msg[i]))
			return false;
	}
	return true;
}


void Handler::passwordMode(bool toggle, Channel *channel, Client *client, char toggleC, char mode, std::string password)
{
	if (channel->isPasswordMode() == toggle && password == channel->getPassword())
		return ;
	else if (!toggle)
		channel->setPassword(toggle, "");
	else
		channel->setPassword(toggle, password);
	std::string messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel->getName() + " " + toggleC + mode + " " + password + "\r\n";
	channel->sendToAll(messageC);
}