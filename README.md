# Accept & poll & epoll
To compile all, run: 
> make all  

Then use the below command to start a server  

> ./server_accept 9000

or

> ./server_poll 9000

or
> ./server_epoll 9000

In another shell, to start a client, run:
> ./client 127.0.0.1 9000

OK! Now you can send messages to the server and receive messages.  
## Client
```shell
➜  accept-poll-epoll git:(master) ✗ ./client 127.0.0.1 9000
connect succeed.
>>hello
receive: hello
>>
```
## Server
```shell
➜  accept-poll-epoll git:(master) ✗ ./server_epoll 9000                                                  
started
serverSocket=6
client(socket=8) connected.
recevied(eventfd=8, size=5):hello
client(eventfd=8) disconnected.
```