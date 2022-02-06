#include <stdio.h>
#include "server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <strings.h>
int main(int argc, char **argv) {
    if (argc != 3) {
        Usage(argv[0]);
        exit(0);
    }
    sqlite3 *db;
    if (sqlite3_open("server.db", &db) != SQLITE_OK) {
        printf("open database failed!\n");
        exit(0);
    }
    int sockfd = create_new_socket(argv[1], argv[2], 10);
    int reuse = 0x01;
    int acceptfd;
    int i;
    printf("server is start %s:%s\n", argv[1], argv[2]);
    printf("waiting for the client connect.........\n");
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr);
    char clientaddr[16];
    unsigned short clientport;
    int epfd = epoll_create(10);
    int max_ready_fd;
    struct epoll_event events[100];
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
    while (1) {
        max_ready_fd = epoll_wait(epfd, events, 100, 5);
        for (i = 0; i < max_ready_fd; i++) {
            if (events[i].data.fd == sockfd) {
                bzero(clientaddr, 16);
                bzero(&clientport, 2);
                acceptfd = accept(sockfd, (struct sockaddr *) &client_addr, &len);
                inet_ntop(AF_INET, (void *) &(client_addr.sin_addr), clientaddr, sizeof(struct sockaddr));
                clientport = ntohs(client_addr.sin_port);
                printf("client %s:%hu is connected\n", clientaddr, clientport);
                event.events = EPOLLIN;
                event.data.fd = acceptfd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, acceptfd, &event) == -1) {
                    perror("epoll_ctl");
                    printf("add the sockfd to epoll base error!\n");
                }
            } else {
                if (events[i].events & EPOLLIN) {
                    if (handler(events[i].data.fd, db) == 0) {
                        getpeername(events[i].data.fd, (struct sockaddr *) &client_addr, &len);
                        inet_ntop(AF_INET, (void *) &(client_addr.sin_addr), clientaddr, sizeof(struct sockaddr));
                        clientport = ntohs(client_addr.sin_port);
                        printf("client %s:%d is disconnected....\n", clientaddr, clientport);
                        event.data.fd = events[i].data.fd;
                        event.events = EPOLLIN;
                        if (epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, &event) == -1) {
                            perror("epoll_ctl");
                            printf("remove from epoll base error!\n");
                        }
                    } else {
                        continue;
                    }
                }
            }
        }
    }
}
