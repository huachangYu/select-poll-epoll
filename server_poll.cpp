#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_POLL_SIZE 1024

int initServer(int port);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("usage: poll_accept port\n");
        return -1;
    }
    int serverSocket = initServer(atoi(argv[1]));
    printf("serverSocket=%d\n", serverSocket);
    if (serverSocket < 0) {
        printf("initServer failed.\n");
        return -1;
    }
    int maxfd;
    struct pollfd fds[MAX_POLL_SIZE];
    for (int i = 0; i < MAX_POLL_SIZE; i++) {
        fds[i].fd = -1;
    }
    fds[serverSocket].fd = serverSocket;
    fds[serverSocket].events = POLLIN;
    maxfd = serverSocket;
    while (1) {
        int infds = poll(fds, maxfd + 1, -1);
        if (infds < 0) {
            printf("poll() failed.\n");
            break;
        } else if (infds == 0) {
            printf("poll() timeout.\n");
            continue;
        }
        for (int eventfd = 0; eventfd <= maxfd; eventfd++) {
            if (fds[eventfd].fd < 0) continue;
            if ((fds[eventfd].revents & POLLIN) == 0) continue;
            fds[eventfd].revents = 0;
            if (eventfd == serverSocket) {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&client, &len);
                if (clientSocket < 0) {
                    printf("accept() failed.\n");
                    continue;
                } 
                printf("client(socket=%d) connected.\n", clientSocket);
                if (clientSocket > MAX_POLL_SIZE) {
                    printf("clientSocket(socket=%d) > MAX_POLL_SIZE(%d).\n", clientSocket, MAX_POLL_SIZE);
                    continue;
                }
                fds[clientSocket].fd = clientSocket;
                fds[clientSocket].events = POLLIN;
                fds[clientSocket].revents = 0;
                if (maxfd < clientSocket) maxfd = clientSocket;
                printf("maxfd=%d.\n", maxfd);
            } else {
                char buf[1024];
                ssize_t isize = read(eventfd, buf, sizeof(buf));
                if (isize <= 0) {
                    printf("client(eventfd=%d) disconnected.\n", eventfd);
                    close(eventfd);
                    fds[eventfd].fd = -1;
                    if (eventfd == maxfd) {
                        for (int ei = maxfd; ei >= 0; ei--) {
                            if (fds[ei].fd != -1) {
                                maxfd = ei;
                                break;
                            }
                        }
                    }
                    printf("maxfd=%d", maxfd);
                } else {
                    buf[isize] = '\0';
                    printf("recevied(evenfd=%d, size=%d):%s\n", eventfd, (int)isize, buf);
                    write(eventfd, buf, isize);
                }
            }
        }
    }
    close(serverSocket);
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