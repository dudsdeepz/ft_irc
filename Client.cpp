#include "Client.hpp"

void Client::setSocket(int socket){clientSocket = socket;}
int Client::getSocket(){return clientSocket;}
const std::vector<std::string>& Client::getChannels(){return channels;}
void Client::setHost(char *ip){host = ip;}
std::string Client::getHost(){return host.c_str();}
void Client::setNick(std::string _nick_){
	this->nick = _nick_;
}

void Client::joinChannel(const std::string& channelName){
	channels.push_back(channelName);
}

std::string Client::getNick(){
	return this->nick;
}


bool Client::isInChannel(std::string channelName){
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) {
        if (*it == channelName) {
            return true;
        }
    }
    return false;
}

Client::Client(){
}

Client::~Client(){
}

Client::Client(const Client &client){
	*this = client;
}
