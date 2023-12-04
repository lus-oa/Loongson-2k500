// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Jinrun Yang, Junchi Ren

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define BUFSIZE 50

int flag = 1;

void *msg_receive(void *arg)
{
	int ret, fd = *((int *)arg);
	char read_buf[BUFSIZE] = { 0 };

	while (flag)
	{
		ret = read(fd, read_buf, sizeof(read_buf));
		if (ret == 1 && read_buf[0] == '\n')
			continue;
		printf("num:%d, recv:%s\r\n", ret, read_buf);
	}
}

int main(int ac,char* av[])
{
	int fd, ret;
	char *filename = "/dev/ttyS1";
	char name[BUFSIZE];
	char buf[BUFSIZE];
	pthread_t pth;

	fd = open(filename, O_RDWR);			
	if(fd < 0){
		perror(filename);
		exit(1);
	}

	ret = pthread_create(&pth, NULL, msg_receive, &fd);
	if (ret != 0)
	{
		printf("test: Error Creating pthread\n");
		return(-1);
	}	

	printf("Please input your nickname:");
	scanf("%s",name);
	int name_len = strlen(name);
	name[name_len] = ':';
	name[name_len+1] = '\0';
	while(getchar() != '\n'){
		continue;
	}
	printf("\n");
	while(fgets(buf,BUFSIZE,stdin) != NULL){
		buf[strlen(buf)] = '\0';
		if (write(fd,name,strlen(name)) == -1 || \
			write(fd,buf,strlen(buf)) == -1 || \
			strncmp(buf, "\\exit", 5) == 0)
		{
			flag = 0;
			break;
		}
	}
	
	close(fd);
}

