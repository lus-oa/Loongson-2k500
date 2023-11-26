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
#include "../aip1944.h"

#define  MAGIC_NUMBER    'k'
#define CMD_qian _IO(MAGIC_NUMBER    ,1)
#define CMD_wei  _IO(MAGIC_NUMBER    ,2)
#define CMD_zhi  _IO(MAGIC_NUMBER    ,3)
#define CMD_neng _IO(MAGIC_NUMBER    ,4)
#define CMD_ke   _IO(MAGIC_NUMBER    ,5)
#define CMD_ji   _IO(MAGIC_NUMBER    ,6)

int main(int argc, char** argv) 
{ 
    int aip_fd,ret; 

	aip_fd = open("/dev/aip", O_RDWR); 
	printf("test: aip_fd=%d\n", aip_fd); 
	if(aip_fd == -1) 
	{ 
		printf("test: Error Opening aip1944\n"); 
		return(-1); 
	}
 
        ret = write(aip_fd, test, sizeof(test));
        sleep(3);
        ret = write(aip_fd, ji, sizeof(ji));
        sleep(3);

    close(aip_fd);

    return 0;
}
