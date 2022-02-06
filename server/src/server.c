#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sqlite3.h>
#include <string.h>
#include <strings.h>
#include <time.h>

void Usage(char *argv) {
    printf("Usage:\n");
    printf("%s: BIND_ADDR BIND_PORT\n", argv);
    printf("example: %s 127.0.0.1 10000\n", argv);
    return;
}

int create_new_socket(char *addr, char *port, int backlog) {
    struct sockaddr_in ADDR;
    ADDR.sin_family = AF_INET;
    ADDR.sin_addr.s_addr = inet_addr(addr);
    ADDR.sin_port = htons(atoi(port));
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int flag = 0x01;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(int)) != 0) {
        perror("setsockopt reuseport");
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) != 0) {
        perror("setsockopt reuseaddr");
    }
    if (bind(sockfd, (struct sockaddr *) &ADDR, sizeof(struct sockaddr)) != 0) {
        perror("bind");
        return -1;
    }
    if (listen(sockfd, backlog) != 0) {
        perror("listen");
        return -1;
    }
    return sockfd;
}

void do_register(int sockfd, Msg_client *msg, sqlite3 *db) {
    time_t register_time;
    char *errmsg;
    char sql[128];
    Msg_server servermsg;
    bzero(&servermsg, sizeof(Msg_server));
    bzero(sql, 128);
    time(&register_time);
    sprintf(sql, "insert into user values('%s','%s','%s');", msg->user, msg->data, ctime(&register_time));
    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
        printf("error:%s", errmsg);
        sqlite3_free(errmsg);
        servermsg.type = 2;
        strcpy(servermsg.data, "user is already exist!\n");
        if (write(sockfd, &servermsg, sizeof(Msg_server)) < 0) {
            printf("send error msg to client is failed!\n");
        }
    } else {
        servermsg.type = 1;
        strcpy(servermsg.data, "register successful!");
        if (write(sockfd, &servermsg, sizeof(Msg_server)) < 0)
            perror("write to sockfd");
        else {
            printf("user %s is register successful!\n", msg->user);
        }
    }
    return;
}

void do_login(int sockfd, Msg_client *msg, sqlite3 *db) {
    char *errmsg;
    char **result;
    char sql[128];
    int Row, Col;
    time_t login_time;
    bzero(sql, 128);
    Msg_server servermsg;
    bzero(&servermsg, sizeof(Msg_server));
    sprintf(sql, "select * from user where username='%s' and password='%s';", msg->user, msg->data);
    time(&login_time);
    if (sqlite3_get_table(db, sql, &result, &Row, &Col, &errmsg) != SQLITE_OK) {
        printf("error:%s", errmsg);
        sqlite3_free(errmsg);
        servermsg.type = 2;
        strcpy(servermsg.data, "username or password invalid!\n");
        if (write(sockfd, &servermsg, sizeof(Msg_server)) <= 0) {
            printf("send error msg to client is failed\n");
        }
        return;
    } else {
        if (Row == 1) {
            bzero(sql, 128);
            sprintf(sql, "select * from last_login where username='%s';", msg->user);
            if (sqlite3_get_table(db, sql, &result, &Row, &Col, &errmsg) != SQLITE_OK) {
                printf("error message:%s\n", errmsg);
                sqlite3_free(errmsg);
            } else {
                if (Row == 0) {
                    bzero(sql, 128);
                    sprintf(sql, "insert into last_login values('%s','%s')", msg->user, ctime(&login_time));

                    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
                        printf("insert last login time to table is failed!\n");
                        printf("error message:");
                        printf("%s\n", errmsg);
                        sqlite3_free(errmsg);
                    }
                } else {
                    sprintf(sql, "update last_login set last_login_time='%s' where username='%s';", ctime(&login_time),
                            msg->user);
                    if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
                        printf("error:%s", errmsg);
                        sqlite3_free(errmsg);
                    }
                }

            }
            servermsg.type = 1;
            if (write(sockfd, &servermsg, sizeof(Msg_server)) < 0)
                printf("send msg to client is failed!\n");
            else {
                printf("user %s is login\n", msg->user);
                fflush(stdout);
            }
        } else {
            servermsg.type = 2;
            strcpy(servermsg.data, "user or password is incorrect");
            if (write(sockfd, &servermsg, sizeof(Msg_server)) < 0) {
                printf("send the errmsg to client is failed");
            }
        }
    }
    return;
}

void do_query_words(int sockfd, Msg_client *msg, sqlite3 *db) {
    char buffer[255], word[64], mean[255], sql[128];
    char *errmsg;
    int i, flag;
    Msg_server MSG;
    time_t cur_time;
    unsigned int word_len = strlen(msg->data);
    bzero(buffer, 255);
    bzero(word, 64);
    bzero(sql, 128);
    FILE *fp = fopen("dict.txt", "r");
    if (fp == NULL) {
        printf("open file error!\n");
        return;
    }
    while (fgets(buffer, 255, fp) != NULL) {
        for (i = 0; i < word_len; i++) {
            if (buffer[i] > msg->data[i] && buffer[i] != ' ')
                break;
        }
        if (strncmp(buffer, msg->data, word_len) == 0 && buffer[word_len] == ' ') {
            i = strlen(msg->data);
            while (buffer[i] == ' ')
                i++;
            flag = i;
            while (buffer[i] != '\0') {
                mean[i - flag] = buffer[i];
                i++;
            }
            mean[i - flag] = '\0';
            MSG.type = 2;
            strcpy(MSG.data, mean);
            printf("found :%s", mean);
            if (write(sockfd, (void *) &MSG, sizeof(Msg_server)) <= 0) {
                perror("write");
            }
            time(&cur_time);
            sprintf(sql, "insert into history values('%s','%s','%s');", msg->user, msg->data, ctime(&cur_time));
            if (sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK) {
                printf("sql exec error:%s", errmsg);
                sqlite3_free(errmsg);
                errmsg = NULL;
            }
            return;
        }
    }
    MSG.type = 2;
    if (write(sockfd, (void *) &MSG, sizeof(Msg_server)) <= 0) {
        perror("write");
    }
    return;
}

void do_query_history(int sockfd, Msg_client *msg, sqlite3 *db) {
    char sql[128];
    char **result;
    int Row, Col;
    char *errmsg;
    Msg_server MSG;
    int a = 0;
    bzero(&MSG, sizeof(Msg_server));
    bzero(sql, 128);
    sprintf(sql, "select * from history where username='%s';", msg->user);
    if (sqlite3_get_table(db, sql, &result, &Row, &Col, &errmsg) != SQLITE_OK) {
        printf("get table error:%s", errmsg);
        sqlite3_free(errmsg);
        strcpy(MSG.data,"no words query history!\n");
    }
    MSG.type = 1;
    for (int i = 3; i < (Row+1) * (Col); i++) {
        if (i % 3 == 0)
            continue;
        for (int j = 0; j < strlen(result[i]); j++) {
            MSG.data[a++] = result[i][j];
        }
        if (i % 3 == 1) {
            MSG.data[a++] = ':';
        }
        /*else {
            MSG.data[a++] = '\n';
        }*/
    }
    MSG.data[a++] = '\0';
    printf("send query history for user:%s\n",msg->user);
    sqlite3_free_table(result);
    int send_byte;
    send_byte = write(sockfd, (void *) &MSG, sizeof(Msg_server));
    if (send_byte <=0){
        printf("send message to cliet is error");
    }
}

int handler(int sockfd, sqlite3 *db) {
    Msg_client msg;
    int read_bytes = read(sockfd, (void *) &msg, sizeof(Msg_client));
    if (read_bytes < 0) {
        perror("read");
        return 0;
    } else if (read_bytes == 0) {
        return 0;
    } else {
        if (msg.type == 1) {
            do_register(sockfd, &msg, db);
        } else if (msg.type == 2) {
            do_login(sockfd, &msg, db);
        } else if (msg.type == 3) {
            do_query_words(sockfd, &msg, db);
        } else if (msg.type == 4) {
            do_query_history(sockfd, &msg, db);
        } else if (msg.type == 5) {
            close(sockfd);
            return 0;
        } else {
            printf("");
        }
    }
    return 1;
}
