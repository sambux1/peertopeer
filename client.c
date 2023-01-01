#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "12829"

int main() {
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    char buf[100];
    int yes = 1;

    printf("Hello from client\n");

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int ret;
    if ((ret = getaddrinfo("127.0.0.1", PORT, &hints, &server_info)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        return 1;
    }

    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    freeaddrinfo(server_info);

    if (p == NULL) {
        fprintf(stderr, "failed to bind");
        exit(1);
    }
    
    for (int i = 0; i < 10; i++) {
        int numbytes;
        if (send(sockfd, "Hello, world!", 13, 0) == -1)
            perror("send");
        
        sleep(1);
    }
    close(sockfd);
    
    //if ((numbytes = recv(sockfd, buf, 99, 0)) == -1) {
    //    perror("recv");
    //    exit(1);
    //}

    //printf("numbytes: %d\n", numbytes);

    //buf[numbytes] = '\0';

    //printf("Received: %s\n", buf);
    /*
    int turn = 0;
    int count = 0;
    while (1) {
        if (turn == 0) {
            if ((numbytes = recv(sockfd, buf, 8, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("%s\n", buf);
            turn = 1;
            sleep(1);
        } else {
            char string[8] = {'C', 'l', 'i', 'e', 'n', 't', ' ', count + '0'};
            if (send(sockfd, string, 8, 0) == -1) {
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
    */
}