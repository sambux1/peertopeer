#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "12345"

int main() {
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int yes = 1;

    printf("Hello from server\n");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int ret;
    if ((ret = getaddrinfo(NULL, PORT, &hints, &server_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return 1;
    }

    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server bind");
            continue;
        }

        break;
    }
    freeaddrinfo(server_info);

    if (p == NULL) {
        fprintf(stderr, "failed to bind");
        exit(1);
    }

    if (listen(sockfd, 10) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Ready for connections\n");

    sin_size = sizeof(client_addr);
    int newfd;
    newfd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
    if (newfd == -1) {
        perror("accept");
        exit(1);
    }

    printf("connection established\n");
    if (send(newfd, "Hello, world!", 13, 0) == -1)
        perror("send");
    
    sleep(1);

    char buf[100];
    int turn = 1;
    int count = 0;
    while (1) {
        if (turn == 0) {
            int numbytes;
            if ((numbytes = recv(newfd, buf, 8, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("%s\n", buf);
            turn = 1;
            sleep(1);
        } else {
            char string[8] = {'S', 'e', 'r', 'v', 'e', 'r', ' ', count + '0'};
            if (send(newfd, string, 8, 0) == -1) {
                perror("send\n");
            }
            count++;
            turn = 0;
        }

        if (count > 9) {
            close(sockfd);
            exit(0);
        }
    }
}