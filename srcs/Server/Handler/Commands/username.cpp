#include "../Handler.hpp"

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