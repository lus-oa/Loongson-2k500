#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <errno.h> 
#include <arpa/inet.h> 
#include <sys/time.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <sys/ioctl.h> 
#include <math.h> 

#define  MAGIC_NUMBER    'k'
#define CMD_standby _IO(MAGIC_NUMBER    ,1)
#define CMD_forward  _IO(MAGIC_NUMBER    ,2)
#define CMD_backward  _IO(MAGIC_NUMBER    ,3)
#define CMD_brake _IO(MAGIC_NUMBER    ,4)

int main(int argc, char** argv) 
{ 
    int fd,ret; 

	fd = open("/dev/motor", O_RDWR); 
	printf("test: fd=%d\n", fd); 
	if(fd == -1) 
	{ 
		printf("test: Error Opening motor\n"); 
		return(-1); 
	}
 

       ioctl(fd,CMD_forward);
       sleep(3);
       ioctl(fd,CMD_standby);
       sleep(3);
       ioctl(fd,CMD_backward);
       sleep(3);
       ioctl(fd,CMD_brake);
       sleep(3);


    close(fd);

    return 0;
}
