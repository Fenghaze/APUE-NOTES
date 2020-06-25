#include <stdlib.h>
#include <stdio.h>
#include "proto.h"
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include<string.h>
int main(int argc, char **argv)
{
    int sd;
    struct msg_st sbuf;
    struct sockaddr_in raddr;   // 远端socket，发送给谁
    // 创建socket
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sd < 0)
    {
        perror("socket()");
        exit(1);
    }
    // 要发送的数据
    strcpy(sbuf.name, "Alan");
    sbuf.math = htonl(98);
    sbuf.chinese = htonl(100);
    // 配置远端的socket
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(RCVPORT)); // 远端端口号
    inet_pton(AF_INET,argv[1], &raddr.sin_addr);
    // 发送数据，发给远端
    if (sendto(sd, &sbuf, sizeof(sbuf), 0, (void *)&raddr, sizeof(raddr)) < 0)
    {
        perror("sendto()");
        exit(1);
    }
    puts("OK!");

    close(sd);

    exit(0);
}