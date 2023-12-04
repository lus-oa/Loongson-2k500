// Copyright (c) 2023 Shandong University
// Copyright (c) 2023 Junchi Ren, Jinrun Yang

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    printf("SocketCAN receive demo made by Shandong university");
	int ret;
	int sockfd, nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
	struct can_filter rfilter;
	sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW); //创建套接字
	strcpy(ifr.ifr_name, "can0");
	ioctl(sockfd, SIOCGIFINDEX, &ifr); //指定 can0 设备
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	ret = bind(sockfd, (struct sockaddr *)&addr,
		   sizeof(addr)); //将套接字与 can0 绑定
	if (ret < 0) {
		perror("bind error!");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	int loopback = 0; //0 表示关闭，1 表示开启(默认)
    setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
	//设置过滤规则
	// setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
	int canfd_on = 1;
	setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));
	while (1) {
		nbytes = read(sockfd, &frame, sizeof(frame));
		//接收报文//显示报文
		if (nbytes > 0) {
			printf("ID=0x%X DLC=%d data[0]=0x%X\n", frame.can_id,
			       frame.can_dlc, frame.data[0]);
			// printf(“ID=0x%X DLC=%d data[0]=0x%X\n”, frame.can_id,
			// frame.can_dlc, frame.data[0]);
		}
	}
	close(sockfd);
	return 0;
}
