#include "../Handler.hpp"

void Handler::usernameCommand(Client *client)
{
	std::string message = client->getMessageBuffer();
	std::istringstream iss(message);
	std::vector<std::string >buffer;
	std::string buffers;
	while (iss >> buffers)
		buffer.push_back(buffers);
	if (buffer.size() <= 1)
	{
		std::string error = ":server 461 * USER :Not enough parameters\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return;
	}
	if (Server::isUserInUse(buffer[1]))
	{
		std::string error = ":server 462 " + client->getNick() + " :You may not register\r\n";
		send(client->getSocket(), error.c_str(), error.size(), 0);
		return ;
	}
	client->setUsername(buffer[1]);
}