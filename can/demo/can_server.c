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

int main() {
    printf("SocketCAN send demo made by Shandong university");
    int ret;
    int sockfd, nbytes;
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_frame frame;
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, "can0");
    ioctl(sockfd, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0){
        perror("bind error!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    int loopback = 0; //0 表示关闭，1 表示开启(默认)
    setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));
    setsockopt(sockfd, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

    frame.can_id = 0x11;
    frame.can_dlc = 2;
    frame.data[0] = 'Y';
    frame.data[1] = 'N';


    while (1) {
        nbytes = write(sockfd, &frame, sizeof(frame));
        printf("nbytes=%d\n", nbytes);
        if (nbytes != sizeof(frame)) {
            printf("Send Error frame write ret:%d\n!",nbytes);
            break; //发送错误,退出
        }
        sleep(1);
    }
    close(sockfd);
    return 0;
}
