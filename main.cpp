#include "server.hpp"

int main()
{
	Server server;
	try{
		server.start();
		int processConnection;
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