// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <ctype.h>
#include <semaphore.h>

#define LISTEN_MAX 100

typedef struct sockaddr SA;
typedef struct sockaddr_in SIN;

int Accept(int s, struct sockaddr *addr, int *addrlen);
int Listen(int s, int backlog);
int Bind(int sockfd, struct sockaddr *my_addr, int addrlen);
int Socket(int domain, int type, int protocol);

typedef struct
{
	int newfd;
	int using;
	int pos;
	pthread_t sockfd;
	SIN src_conn;
}ClientInfo;

void *task_read(void *arg)
{
	ClientInfo *client = (ClientInfo *)arg;
	char *client_ip = inet_ntoa(client->src_conn.sin_addr);
	while (1)
	{
		//读取
		char read_buff[512] = {0};
		int len = read(client->newfd, read_buff, sizeof(read_buff));
		if (len > 0)
		{
			printf("ClientIP:%s  序号:%d  服务器读取：%s\n", client_ip, client->pos, read_buff);
		}
		else
		{
			//read读取不到会堵塞
			printf("ClientIP:%s  客户端断开连接  序号%d开放接入\n", client_ip, client->pos);
			close(client->newfd);
			client->using = 0;
			pthread_exit(NULL);
		}
	}
}

void *task_write(void *arg)
{
	ClientInfo *client = (ClientInfo *)arg;
	char *client_ip = inet_ntoa(client->src_conn.sin_addr);
}

void *task_client(void *arg)
{
	ClientInfo *client = (ClientInfo *)arg;
	char *client_ip = inet_ntoa(client->src_conn.sin_addr);
	pthread_t read_id = 0;
	pthread_t write_id = 0;
	pthread_create(&read_id, NULL, task_read, (void *)client);
	//pthread_create(&write_id, NULL, task_write, (void *)client);

	pthread_join(read_id, NULL);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		printf("参数错误\n");
		exit(EXIT_FAILURE);
	}
	//创建套接字
	int socketfd = Socket(AF_INET, SOCK_STREAM, 0); //ipv4 TCP
	printf("socketfd:%d\n", socketfd);
	//编写服务器信息
	SIN serverinfo = {0};
	serverinfo.sin_family = AF_INET;	//ipv4
	serverinfo.sin_port = htons(atoi(argv[2]));	//参数传递端口号port
	serverinfo.sin_addr.s_addr = inet_addr(argv[1]);	//参数传递ip地址
	//解决客户端频繁连接退出
	int optval = 1;
	setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	//绑定信息
	Bind(socketfd, (SA *)&serverinfo, sizeof(SA));
	//监听
	Listen(socketfd, LISTEN_MAX);
	
	ClientInfo client_list[LISTEN_MAX] = {0};
	while (1)
	{
		//等待客户端连接
		ClientInfo *client = NULL;
		for (int i = 0; i < LISTEN_MAX; i++)
		{
			if (client_list[i].using == 0)
			{
				client = &client_list[i];	
				client->using = 1;
				client->pos = i;
				break;
			}
		}
		if (client == NULL)
		{
			printf("服务器已满，等待用户退出。。。\n");
			sleep(3);
			continue;
		}
		int client_addrlen = sizeof(SA);
		client->newfd = Accept(socketfd, (SA *)&client->src_conn, &client_addrlen);
		//创建线程
		client->sockfd = 0;
		pthread_create(&client->sockfd, NULL, task_client, (void *)client);
		printf("客户端序号:%d  接入IP:%s\n", client->pos, inet_ntoa(client->src_conn.sin_addr));
	}
	return 0;
}

int Socket(int domain, int type, int protocol)
{
	int val = socket(domain, type, protocol);
	if (val < 0)
	{
		perror("socket");
		return -1;
	}
	return val;
}

int Bind(int sockfd, struct sockaddr *my_addr, int addrlen)
{
	int val = bind(sockfd, my_addr, addrlen);
	if (val < 0)
	{
		perror("bind");
		return -1;
	}
	return val;
}

int Listen(int s, int backlog)
{
	int val = listen(s, backlog);
	if (val < 0)
	{
		perror("listen");
		return -1;
	}
	return val;
}

int Accept(int s, struct sockaddr *addr, int *addrlen)
{
	int val = accept(s, addr, addrlen);
	if (val < 0)
	{
		perror("accept");
		return -1;
	}
	return val;
}
