#include "../Handler.hpp"

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
		std::string message = "Welcome\t" + client->getNick() + " !\r\n";
		send(client->getSocket(), message.c_str(), message.size(), 0);
	}
	else
	{
		std::string error = ":server 464 " + client->getNick() + " :Password incorrect\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
}