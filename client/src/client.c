#include "client.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

void Usage(char *argu) {
    printf("Usage:\n");
    printf("%s SERVERADDR SERVERPORT\n", argu);
    printf("exit!\n");
    return;
}

int conser(char *addr, char *port) {
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, addr, &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(atoi(port));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) != 0) {
        perror("connect");
        exit(0);
    }
    return sockfd;
}

void mainmenu(void) {
    printf("******welcome to online directory!******\n");
    printf("***           1.register             ***\n");
    printf("***           2.login                ***\n");
    printf("***           3.quit                 ***\n");
    printf("please choose:");
    return;
}

void logined(int sockfd, char *user) {
    int choose;
    Msg_client data;
    bzero(&data, sizeof(Msg_client));
    while (1) {
        printf("***           1.query words                   ***\n");
        printf("***           2.queryt history                ***\n");
        printf("***           3.exit                          ***\n");
        printf("please input:");
        scanf("%d", &choose);
        getchar();
        switch (choose) {
            case 1:
                strcpy(data.user, user);
                data.type = 3;
                printf("please input the word that you want to query:");
                scanf("%s", data.data);
                query(sockfd, &data);
                bzero(&data, sizeof(Msg_client));
                break;
            case 2:
                data.type = 4;
                if (strcmp("root", user) == 0) {
                    printf("please input the history user that you want query:");
                    scanf("%s", data.user);

                } else {
                    strcpy(data.user, user);
                }
                query_history(sockfd, &data);
                bzero(&data, sizeof(Msg_client));
                break;
            case 3:
                return;
            default:
                printf("invalid input!\n");
                break;
        }
    }
}

void quit_system(int sockfd) {
    Msg_client msg;
    bzero(&msg, sizeof(Msg_client));
    msg.type = 5;
    if (write(sockfd, (void *) &msg, sizeof(Msg_client) < 0)) {
        printf("send quit message to server is failed\n");
        return;
    } else {
        close(sockfd);
        printf("quit!\n");
        return;
    }
}

void do_login(int sockfd) {
    Msg_client msg;
    Msg_server servermsg;
    bzero(&msg, sizeof(msg));
    bzero(&servermsg, sizeof(Msg_server));
    msg.type = 2;
    printf("please input your username:");
    scanf("%s", msg.user);
    printf("please input your password:");
    scanf("%s", msg.data);
    if (write(sockfd, (void *) &msg, sizeof(Msg_client)) <= 0) {
        printf("send the message to server failed");
        close(sockfd);
        return;
    }
    if (read(sockfd, (void *) &servermsg, sizeof(Msg_server)) <= 0) {
        printf("recv from server failed!\n");
        close(sockfd);
        return;
    }
    if (servermsg.type == 1) {
        printf("login successful\n");
        logined(sockfd, msg.user);
        return;
    }
    if (servermsg.type == 2) {
        printf("login failed please check your username or password!\n");
        printf("server:%s",servermsg.data);
    }
    bzero(&servermsg,sizeof(Msg_server));
    return;
}

void do_register(int sockfd) {
    Msg_client msg;
    Msg_server servermsg;
    bzero(&msg, sizeof(Msg_client));
    msg.type = 1;
    while (1) {
        printf("please input username:");
        scanf("%s", msg.user);
        printf("please input password:");
        scanf("%s", msg.data);
        if (write(sockfd, (void *) &msg, sizeof(msg)) <= 0)
            printf("send require message to remote server is failed!\n");
        else {
            if (read(sockfd, (void *) &servermsg, sizeof(Msg_server)) < 0)
                printf("recv from remote server message is failed\n");
            else {
                if (servermsg.type == 1) {
                    printf("register is successful!\n");
                } else if (servermsg.type == 2) {
                    printf("register is failed:%s\n", servermsg.data);
                } else {
                    printf("server internal error!\n");
                }
            }
        }
        return;
    }
}

void query(int sockfd, Msg_client *data) {
    Msg_server Msg;
    bzero(&Msg, sizeof(Msg_server));
    if (write(sockfd, data, sizeof(Msg_client)) <= 0) {
        printf("query fail,please check your network connection\n");
    } else {
        if (read(sockfd, &Msg, sizeof(Msg)) > 0) {
            if (Msg.type == 1) {
                printf("query fail!\n");
                printf("%s\n", Msg.data);
            }
            if (Msg.type == 2) {
                printf("query successful\n");
                printf("%s", Msg.data);
            } else {
                printf("server internal fail!\n");
            }
        } else {
            printf("no recv from server\n");
        }
    }
    return;
}

void query_history(int sockfd, Msg_client *data) {
    if (write(sockfd, (void *) data, sizeof(Msg_client)) <= 0) {
        printf("send message to server is error!\n");
    }
    Msg_server msg;
    bzero(&msg, sizeof(Msg_server));
    int rec_byte;
    rec_byte = read(sockfd, (void *) &msg, sizeof(Msg_server));
    if (rec_byte <= 0) {
        printf("recv message from server error!\n");
    } else {
        if (msg.type == 1) {
            printf("history:\n%s", msg.data);
            bzero(&msg, sizeof(Msg_server));
        } else if (msg.type == 2) {
            printf("query history fail!\n");
            printf("server:%s",msg.data);
        } else {
            printf("server internal error!\n");
        }
        bzero(&msg,sizeof(Msg_server));
    }
    return;
}
