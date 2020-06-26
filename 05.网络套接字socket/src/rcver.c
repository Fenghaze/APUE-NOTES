#include<stdlib.h>
#include<stdio.h>
#include "proto.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include<arpa/inet.h>
int main()
{
    int sd;
    struct sockaddr_in laddr, radrr; // 本地端socket，远端socket
    struct msg_st rbuf;
    socklen_t radrr_len;
    char ipstr[40];
    // 创建socket
    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(RCVPORT));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);

    // 为服务端绑定一个地址与端口号,方便客户端发送请求
    if(bind(sd, (void *)&laddr, sizeof(laddr)))
    {
        perror("bind()");
        exit(1);
    }

    radrr_len =  sizeof(radrr);

    while(1)
    {
        // 接收数据，从远端接收
        recvfrom(sd, &rbuf, sizeof(rbuf), 0, (void *)&radrr, &radrr_len);
        inet_ntop(AF_INET, &radrr.sin_addr, ipstr, 40);
        printf("-------MESSAGE FROM %s:%d-----\n",
                ipstr, ntohs(radrr.sin_port));
        printf("NAME=%s\n", rbuf.name);
        printf("MATH=%s\n", rbuf.math);
        printf("CHINESE=%s\n", rbuf.chinese);
    }
    close(sd);


    exit(0);
}