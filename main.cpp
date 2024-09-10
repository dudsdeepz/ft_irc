#include "./srcs/Server/server.hpp"
#include "./srcs/Server/Handler/Handler.hpp"

int main(int ac, char **av)
{
	Server server;
	(void)av;
	if (ac != 3)
		return 1;
	try{
		server.setPort(av[1]);
		server.setPassword(av[2]);
		server.start();
		int processConnection;
		Handler::registerCommands();
		signal(SIGINT, Server::ctrlChandler);
		while (true)
		{
			processConnection = epoll_wait(server.getEpFD() , server.getEvents(), 64, -1);
			for (int i = 0; i < processConnection;  i++){
				if (server.getEventFd(i) == server.getServerSocket())
					server.lobby();
				else
					server.processData(i);	
			}
		}
	}
	catch(const std::exception& e){
		std::cerr << e.what() << std::endl;
		return 1;
	}
}