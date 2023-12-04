// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

typedef struct sockaddr SA;
typedef struct sockaddr_in SIN;

typedef struct {
	int socketfd;
	SIN server_conn;
}ServerInfo;

void *task_read(void *arg)
{
	ServerInfo *server = (ServerInfo *)arg;
	char *server_ip = inet_ntoa(server->server_conn.sin_addr);
	while (1)
	{
		char read_buff[512] = {0};
		int len = read(server->socketfd, read_buff, sizeof(read_buff));
		if (len > 0)
		{
			printf("ServerIP:%s  接收信息：%s\n", server_ip, read_buff);
		}
		else
		{
			printf("ServerIP:%s 已断开连接\n", server_ip);
			pthread_exit(NULL);
		}
	}
}

int main(int argc,char *argv[])
{
	int socketfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (argc < 3)
	{
		printf("arg error!\n");
		return -1;
	}

	ServerInfo server = {0};
	server.socketfd = socketfd;
	server.server_conn.sin_family = AF_INET;	//ipv4
	server.server_conn.sin_port = htons(atoi(argv[2]));	//参数传递端口号port
	server.server_conn.sin_addr.s_addr = inet_addr(argv[1]);	//参数传递ip地址
	
	connect(socketfd, (SA *)&server.server_conn, sizeof(SA));

	pthread_t id = 0;
	pthread_create(&id, NULL, task_read, (void *)&server);
	while (1)
	{
		char write_buff[512] = {0};
		scanf("%s", write_buff);
		write(socketfd, write_buff, strlen(write_buff));
	}
	return 0;
}

