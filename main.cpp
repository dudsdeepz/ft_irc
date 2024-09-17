#include "./srcs/Server/server.hpp"
#include "./srcs/Server/Handler/Handler.hpp"

int main(int ac, char **av)
{
	if (ac != 3)
		return 1;
	try{
		Server server;
		int processConnection;
		server.setPort(av[1]);
		server.setPassword(av[2]);
		server.start();
		Handler::registerCommands();
		signal(SIGINT, Server::ctrlChandler);
		signal(SIGPIPE, SIG_IGN);
		while (true)
		{
			processConnection = epoll_wait(server.getEpFD() , server.getEvents(), 64, -1);
			for (int i = 0; i < processConnection;  ++i){
				if (server.getEvents()[i].data.fd && EPOLLIN)
				{
					if (server.getEventFd(i) == server.getServerSocket())
						server.lobby();
					else
						server.processData(i);
				}
				else if (server.getEvents()[i].data.fd && EPOLLERR)
					continue ;
				else if (server.getEvents()[i].data.fd && EPOLLOUT)
					continue ;
			}
		}
	}
	catch(const std::exception& e){
		std::cerr << e.what() << std::endl;
		return 1;
	}
}