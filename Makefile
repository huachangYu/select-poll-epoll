cc = g++

client:
	$(cc) -g -o client client.cpp

accept:
	$(cc) -g -o server_accept server_accept.cpp

poll:
	$(cc) -g -o server_poll server_poll.cpp

epoll:
	$(cc) -g -o server_epoll server_epoll.cpp

all:
	make client
	make accept
	make poll
	make epoll

clean:
	rm client server_accept poll