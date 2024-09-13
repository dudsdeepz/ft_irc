#include "Client.hpp"
#include <cstdlib>

void Client::setSocket(int socket){clientSocket = socket;}

std::string Client::getNick(){
	return this->nick;
}

std::string Client::getMessageBuffer(){
	return messageBuffer;
}

int Client::getSocket(){return clientSocket;}

std::vector<std::string>& Client::getChannels(){return channels;}

std::string Client::getHost(){return host.c_str();}

std::string Client::getUsername(){
	return this->username;
}

bool Client::getAuthentication(){return authenticated;}

void Client::setAuthentication(bool yesorno){authenticated = yesorno;}

void Client::setMessageBuffer(std::string msg){
	messageBuffer = msg;
}

void Client::setHost(char *ip){host = ip;}

void Client::setNick(std::string _nick_){
	this->nick = _nick_;
}

void Client::setUsername(std::string _nick_){
	this->username = _nick_;
}

bool Client::isInChannel(std::string channelName){
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); it++) {
        if (*it == channelName) {
            return true;
        }
    }
    return false;
}

void Client::leaveChannel(std::string channelName){
	channels.erase(std::remove(channels.begin(), channels.end(), channelName), channels.end());
}

Client::Client(){
	authenticated = false;
	nick = "";
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
		if (tempChannel->isOperator(nick))
			tempChannel->removeToOperators(nick);
		tempChannel->removeFromList(nick);
		tempChannel->sendToAll(message);
	}
}

void Client::appendData(const std::string& data){
	tempData += data;	
}

std::string& Client::getData(){
	return tempData;
}

void Client::clearData(){
	tempData.clear();
}