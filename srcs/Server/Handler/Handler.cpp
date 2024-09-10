#include "Handler.hpp"
#include "../server.hpp"

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

void Handler::privmsgCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	Server::sendMessageToChannel(client->getNick(), message);
}

void Handler::capCommand(Client *client)
{
	(void)client;
}

void Handler::quitSignal(Client* client)
{
	client->clientQUittingMsg();
	epoll_ctl(Server::getEpFD(), EPOLL_CTL_DEL, client->getSocket(), NULL);
	close(client->getSocket());
	std::cout << "Client disconnected: " << (client)->getNick() << std::endl;
	Server::clientPoolErase(client->getSocket());
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

void Handler::modeInviteOnly(bool toggle, Channel *channel, Client *client, char toggleC, char mode)
{
	if (toggle == channel->isInviteOnly())
		return;
	else
		channel->switchInviteMode(toggle);
	std::string messageC = ":" + client->getNick() + "!" + client->getUsername() + "@" + client->getHost() + " MODE " + channel->getName() + " " + toggleC + mode + "\r\n";
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