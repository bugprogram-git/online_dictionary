#ifndef __SERVER_H_
#define __SERVER_H_
#include <sqlite3.h>

typedef struct
{
	int type;
	//1.register
	//2.login
	//3.query words
	//4.query history record
	char user[20];
	char data[200];
}Msg_client;//定义客户端发送消息结构体
typedef struct
{
	int type;
	//1.successful
	//2.error
	char data[200];
	//当 type ==1 data为查询数据或者无效数据
	//当 type == 2 data为错误信息
}Msg_server;//定义服务端发送信息结构体
void Usage(char *argv);//提示使用方法
int create_new_socket(char *addr,char *port,int backlog);//创建一个监听在本地的socket;
void do_register(int sockfd,Msg_client *msg,sqlite3 *db);//处理客户端注册请求;
void do_login(int sockfd,Msg_client *msg,sqlite3 *db);//处理客户端登录请求;
void do_query_words(int sockfd,Msg_client *msg,sqlite3 *db);//处理客户端查询单词请求;
void do_query_history(int sockfd,Msg_client *msg,sqlite3 *db);//处理客户端查询历史记录请求;
int handler(int sockfd,sqlite3 *db);//处理客户端请求;

#endif
