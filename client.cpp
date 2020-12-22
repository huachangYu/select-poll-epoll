#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage: client ip port\n");
        return -1;
    }
    int socketfd;
    struct sockaddr_in serverAddr;
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket() failed.\n");
        return -1;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(atoi(argv[2]));

    if (connect(socketfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) != 0) {
        printf("connect(%s, %s) failed.\n", argv[1], argv[2]);
        close(socketfd);
        return -1;
    }
    printf("connect succeed.\n");
    
    char buf[1024];
    while (1) {
        memset(buf, 0, sizeof(buf));
        printf(">>");
        scanf("%s", buf);
        if (write(socketfd, buf, strlen(buf)) <= 0) {
            printf("write() failed.\n");
            close(socketfd);
            return -1;
        }
        memset(buf, 0, sizeof(buf));
        if (read(socketfd, buf, sizeof(buf)) <= 0) {
            printf("read() failed.\n");
            close(socketfd);
            return -1;
        }
        printf("receive: %s\n", buf);
    }
    return 0;
}