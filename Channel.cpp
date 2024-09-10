#include "Channel.hpp"


void Channel::setName(char *name){name_ = name;}

std::string Channel::getName(){return name_;}

void Channel::removeFromList(std::string message){
	   namesList.erase(std::remove(namesList.begin(), namesList.end(), message),namesList.end());
}

void Channel::sendToAll(std::string message)
{
	std::vector<Client *>tempPool = Server::getClientPool();
	for (std::vector<Client *>::iterator it = tempPool.begin(); it != tempPool.end(); it++)
	{
		if ((*it)->isInChannel(name_))
			send((*it)->getSocket(), message.c_str(), message.size(), 0);
	}
}

std::string Channel::retrieveNamesList(std::string clientName){
	std::string formattedNamesList = ":server 353 " + clientName + " = " + name_ + " :";

    for (std::vector<std::string>::const_iterator it = namesList.begin(); it != namesList.end(); ++it) {
		if (std::find(operators.begin(), operators.end(), *it) != operators.end())
			formattedNamesList += "@";
        formattedNamesList += *it + " ";
    }
    if (!namesList.empty()) {
        formattedNamesList.erase(formattedNamesList.size() - 1);
    }
	formattedNamesList += "\r\n";
	std::cout << "|" << formattedNamesList << "|" << std::endl;
    return formattedNamesList;
}

std::vector<std::string> Channel::retrieveOpList(){
	std::cout << operators.size() << "retrieveOplist" << std::endl;
	return operators;
}

std::vector<std::string> Channel::getNameslist(){
	std::cout << namesList.size() << std::endl;
	return namesList;
}

Channel::Channel(std::string name, std::string op) : name_(name)
{
	limit = INT_MAX;
	limit_on = false;
	invite_only = false;
	password = "";
	operators.push_back(op);
	topicOpOnly = true;
	password_mode = false;
}

void Channel::addToNamesList(std::string name){
	namesList.push_back(name);
	std::cout << namesList.size() << "addToNamesLIst" <<std::endl;
}

void Channel::setTopic(std::string msg){
	topic = msg;
}

std::string Channel::getTopic(){
	return topic;
}

void Channel::addToOperators(std::string name){
	operators.push_back(name);
}

void Channel::removeToOperators(std::string name){
	operators.erase(std::remove(operators.begin(), operators.end(), name),operators.end());
}

bool Channel::isOperator(std::string name){
	if (std::find(operators.begin(), operators.end(), name) != operators.end())
		return true;
	return false;
}

void Channel::removeOnNamesList(std::string name){
	namesList.erase(std::remove(namesList.begin(), namesList.end(), name),namesList.end());
	std::cout << namesList.size() << "removeOnNamesList" <<std::endl;
}

void Channel::addToInviteList(std::string nick){
	inviteList.push_back(nick);
}

void Channel::removeFromInviteList(std::string nick)
{
	inviteList.erase(std::remove(inviteList.begin(), inviteList.end(), nick),inviteList.end());
}

void Channel::switchInviteMode(bool mode){
	invite_only = mode;
}

bool Channel::isInviteOnly(){
	return invite_only;
}

bool Channel::isUserInInviteList(std::string nick){
	if (std::find(inviteList.begin(), inviteList.end(), nick) != inviteList.end())
		return true;
	return false;
}

bool Channel::isUserOnThisChannel(std::string name)
{
	if (std::find(namesList.begin(), namesList.end(), name) != namesList.end())
		return true;
	return false;
}

bool Channel::isTopicOpOnly()
{
	return topicOpOnly;
}

void Channel::changeTopicOpOnly(bool mod){
	topicOpOnly = mod;
}

bool Channel::isLimitReached(){
	if (namesList.size() == limit)
		return true;
	return false;
}

void Channel::setLimit(int lim, bool mod){
	limit_on = mod;
	limit = lim;
}

bool Channel::getLimitmod(){
	return limit_on;
}

int Channel::getLimitNum(){
	return limit;
}

std::string Channel::getPassword(){
	return password;
}

void Channel::setPassword(bool mode, std::string pas){
	password_mode = mode;
	password = pas;
}

bool Channel::isPasswordMode()
{
	return password_mode;
}