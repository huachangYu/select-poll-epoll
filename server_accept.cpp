#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int initServer(int port);

int main(int argc, char *argv[]) {
    printf("started\n");
    if (argc != 2) {
        printf("usage: server_accept port\n");
        return -1;
    }
    int serverSocket = initServer(atoi(argv[1]));
    printf("serverSocket=%d\n", serverSocket);
    if (serverSocket < 0) {
        printf("initServer failed.\n");
        return -1;
    }
    fd_set readfdset;
    int maxfd;
    FD_ZERO(&readfdset);
    FD_SET(serverSocket, &readfdset);
    maxfd = serverSocket;
    while (1) {
        fd_set tmpfdset = readfdset;
        int infds = select(maxfd + 1, &tmpfdset, NULL, NULL, NULL);
        if (infds < 0) {
            printf("select() failed.\n");
            break;
        } else if (infds == 0) {
            printf("select() timeout.\n");
            continue;
        }
        for (int eventfd = 0; eventfd <= maxfd; eventfd++) {
            if (FD_ISSET(eventfd, &tmpfdset) <= 0) continue;
            if (eventfd == serverSocket) {
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientSocket = accept(serverSocket, (struct sockaddr*)&client, &len);
                if (clientSocket < 0) {
                    printf("accept() failed.\n");
                    continue;
                }
                printf("client(socket=%d) connected ok.\n", clientSocket);
                FD_SET(clientSocket, &readfdset);
                if (maxfd < clientSocket) maxfd = clientSocket;
            } else {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                ssize_t isize = read(eventfd, buf, sizeof(buf));
                if (isize <= 0) {
                    printf("client(eventfd=%d) disconnected.\n",eventfd);
                    close(eventfd);
                    FD_CLR(eventfd, &readfdset);
                    if (eventfd == maxfd) {
                        for (int f = maxfd; f > 0; f--) {
                            if (FD_ISSET(f, &readfdset)) {
                                maxfd = f;
                                break;
                            }
                        }
                    }
                    printf("maxfd=%d\n",maxfd);
                } else {
                    printf("recv(eventfd=%d,size=%d):%s\n", eventfd, (int)isize, buf);
                    write(eventfd, buf, strlen(buf));
                }
            }
        }

    }
    close(serverSocket);
    printf("ended\n");
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