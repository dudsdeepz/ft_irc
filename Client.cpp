#include "Client.hpp"
#include <cstdlib>

void Client::setSocket(int socket){clientSocket = socket;}

int Client::getSocket(){return clientSocket;}

std::vector<std::string>& Client::getChannels(){return channels;}

void Client::setHost(char *ip){host = ip;}

std::string Client::getHost(){return host.c_str();}

bool Client::getAuthentication(){return authenticated;}

void Client::setAuthentication(bool yesorno){authenticated = yesorno;}

void Client::setMessageBuffer(std::string msg){
	messageBuffer = msg;
}

std::string Client::getMessageBuffer(){
	return messageBuffer;
}

void Client::setNick(std::string _nick_){
	this->nick = _nick_;
}

std::string Client::getNick(){
	return this->nick;
}

void Client::setUsername(std::string _nick_){
	this->username = _nick_;
}

std::string Client::getUsername(){
	return this->username;
}
bool Client::isInChannel(std::string channelName){
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++) {
        if (*it == channelName) {
            return true;
        }
    }
    return false;
}

Client::Client(){
	authenticated = false;
}

Client::~Client(){
}

Client::Client(const Client &client){
	*this = client;
}


void Client::clientQUittingMsg(){
    std::string message = ":" + nick + "!" + username + "@" + host + " " + "QUIT :Quit: Leaving" + "\r\n";
	std::cout << message << std::endl;
	for(std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		Channel* tempChannel = Server::getChannel(*it);
		tempChannel->removeFromList(nick);
		tempChannel->sendToAll(message);
	}
}

void Client::leaveChannel(std::string channelName){
	channels.erase(std::remove(channels.begin(), channels.end(), channelName), channels.end());
}