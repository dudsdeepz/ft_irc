#include "../Handler.hpp"

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