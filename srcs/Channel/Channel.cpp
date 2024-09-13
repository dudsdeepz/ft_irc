#include "Channel.hpp"

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

Channel::~Channel(){
}

//GET

std::string Channel::getName(){return name_;}

std::vector<std::string> Channel::getNameslist(){
	return namesList;
}

std::string Channel::getTopic(){
	return topic;
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

//SET

void Channel::setName(char *name){name_ = name;}

void Channel::setTopic(std::string msg){
	topic = msg;
}

void Channel::setPassword(bool mode, std::string pas){
	password_mode = mode;
	password = pas;
}

void Channel::setLimit(int lim, bool mod){
	limit_on = mod;
	limit = lim;
}

//HELP FUNCTIONS

void Channel::removeFromList(std::string message){
	   namesList.erase(std::remove(namesList.begin(), namesList.end(), message),namesList.end());
}

bool Channel::isUserOnThisChannel(std::string name)
{
	if (std::find(namesList.begin(), namesList.end(), name) != namesList.end())
		return true;
	return false;
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
    return formattedNamesList;
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

//OPERATORS

std::vector<std::string> Channel::retrieveOpList(){
	return operators;
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

void Channel::replaceOnOpList(std::string name, std::string oldname){
	for (std::vector<std::string>::iterator it = operators.begin(); it != operators.end(); ++it) {
        if (*it == oldname)
		{
			*it = name;
			return ;
		}	
	}
}
//NAMELIST

void Channel::addToNamesList(std::string name){
	namesList.push_back(name);
}

void Channel::removeOnNamesList(std::string name){
	namesList.erase(std::remove(namesList.begin(), namesList.end(), name),namesList.end());
}

//INVITE MOD
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

void Channel::replaceOnNamesList(std::string name, std::string oldname){
	for (std::vector<std::string>::iterator it = namesList.begin(); it != namesList.end(); ++it) {
        if (*it == oldname)
		{
			*it = name;
			return ;
		}
	}
}

// TOPIC MODE

bool Channel::isTopicOpOnly()
{
	return topicOpOnly;
}

void Channel::changeTopicOpOnly(bool mod){
	topicOpOnly = mod;
}

//LIMIT MODE
bool Channel::isLimitReached(){
	if (namesList.size() == limit)
		return true;
	return false;
}

// PASSWORD MODE
bool Channel::isPasswordMode()
{
	return password_mode;
}