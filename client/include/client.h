#ifndef __CLIENT_H_
#define __CLIENT_H_
typedef struct
{
	int type;
	char user[20];
	char data[200];
}Msg_client;//定义客户端消息结构体
typedef struct
{
	int type;
//1.提示消息
//2.数据
	char data[200];
}Msg_server;//定义服务端消息结构体
void mainmenu(void);//主菜单;
void Usage(char *argu);//提示使用方法
int conser(char *addr,char *port);//连接远程服务器
void do_register(int sockfd);//注册功能函数
void do_login(int sockfd);//登录功能函数
void quit_system(int sockfd);//退出函数;
void logined(int sockfd,char *user);//登录后的功能函数
void query(int sockfd,Msg_client *data);//查询单词函数
void query_history(int sockfd,Msg_client *data);//查询历史记录函数


#endif
