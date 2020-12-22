#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 100

int initServer(int port);

int main(int argc, char *argv[]) {
    printf("started\n");
    if (argc != 2) {
        printf("usage: server_epoll port\n");
        return -1;
    }
    int serverSocket = initServer(atoi(argv[1]));
    printf("serverSocket=%d\n", serverSocket);
    if (serverSocket < 0) {
        printf("initServer failed.\n");
        return -1;
    }
    int epollfd;
    epollfd = epoll_create(1);
    struct epoll_event ev;
    ev.data.fd = serverSocket;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &ev);
    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int infds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (infds < 0) {
            printf("epoll_wait() failed.\n");
            break;
        } else if (infds == 0) {
            printf("epoll_wait() timeout.\n");
            break;
        }
        for (int eventfd = 0; eventfd < infds; eventfd++) {
            if (events[eventfd].data.fd == serverSocket && (events[eventfd].events & EPOLLIN)) {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&client, &len);
                if (clientSocket < 0) {
                    printf("accept() failed.\n");
                    continue;
                }
                memset(&ev, 0, sizeof(struct epoll_event));
                ev.data.fd = clientSocket;
                ev.events = EPOLLIN;
                epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSocket, &ev);
                printf("client(socket=%d) connected.\n", clientSocket);
            } else if (events[eventfd].events & EPOLLIN) {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                ssize_t isize = read(events[eventfd].data.fd, buf, sizeof(buf));
                if (isize <= 0) {
                    printf("client(eventfd=%d) disconnected.\n", events[eventfd].data.fd);
                    memset(&ev,0,sizeof(struct epoll_event));
                    ev.events = EPOLLIN;
                    ev.data.fd = events[eventfd].data.fd;
                    epoll_ctl(epollfd,EPOLL_CTL_DEL,events[eventfd].data.fd,&ev);
                    close(events[eventfd].data.fd);
                } else {
                    printf("recevied(eventfd=%d, size=%d):%s\n", events[eventfd].data.fd, (int)isize, buf);
                    write(events[eventfd].data.fd, buf, strlen(buf));
                }
            }
        }
    }
    close(epollfd);
    printf("end.\n");
    return 0;
}

int initServer(int port) {
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0) {
        printf("socket() failed.\n");
        return -1;
    }
    
    int opt = 1;
    unsigned int len = sizeof(opt);
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, len);
    setsockopt(socketfd, SOL_SOCKET, SO_KEEPALIVE, &opt, len);
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (bind(socketfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("bind() failed.\n");
        close(socketfd);
        return -1;
    }

    if (listen(socketfd, 100) != 0) {
        printf("listen() failed.\n");
        close(socketfd);
        return -1;
    }
    return socketfd;
}