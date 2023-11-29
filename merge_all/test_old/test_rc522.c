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
#include "../rc522.h"

int main(int argc, char** argv) 
{ 
	int rc522_fd; 
	int i, read_num, write_num; 
	char r[256];
	char a[16]; 
	rc522_fd = open("/dev/rfid_dev", O_RDWR); 
	printf("test: rc522_fd=%d\n", rc522_fd); 
	if(rc522_fd == -1) 
	{ 
		printf("test: Error Opening rc522\n"); 
		return(-1); 
	} 
	printf("test: wait begin\n"); 
	sleep(1); //wait 
	printf("test: wait done\n"); 

	ioctl(rc522_fd, GET_ID, &(a[0]));//参数3：选第0块 */
	printf("%d%d%d%d", a[0],a[1],a[2],a[3]); 
	while (1)
	{ 
		read_num = read(rc522_fd, r, 0); 
		printf("read_num=%d\n", read_num); 
		for (i = 0; i < read_num; i++)
		{
			printf("%x ", r[i]);
		}
		printf("\n");
		if(read_num > 0){ 
			write_num = write(rc522_fd, "\1\1\1", 3);
			printf("write_num=%d\n", write_num); 
			
			read_num = read(rc522_fd, r, 0); 
			printf("read again read_num=%d\n", read_num); 
			for (i = 0; i < read_num; i++)
			{
				printf("%x ", r[i]);
			}
			printf("\n");
			break;
		} 
		sleep(1); 
	}
	close(rc522_fd);
	return 0; 
} 

