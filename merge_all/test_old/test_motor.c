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

#define MOTOR_MAGIC	'k'
#define CMD_standby	_IO(MOTOR_MAGIC, 1)
#define CMD_forward	_IO(MOTOR_MAGIC, 2)
#define CMD_backward	_IO(MOTOR_MAGIC, 3)
#define CMD_brake	_IO(MOTOR_MAGIC, 4)

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
       usleep(1000 * 500);
       ioctl(fd,CMD_standby);
       usleep(1000 * 500);
       ioctl(fd,CMD_backward);
       usleep(1000 * 500);
       ioctl(fd,CMD_brake);
       usleep(1000 * 500);


	close(fd);

    return 0;
}
