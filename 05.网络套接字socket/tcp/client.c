#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include "proto.h"
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int sd;
    struct sockaddr_in raddr;
    FILE *fp;
    long long stamp;
    // 1、取得socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }

    // 2、请求连接,连接远端socket
    raddr.sin_family = AF_INET;
    raddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, argv[1], &raddr.sin_addr);
    if(connect(sd, (void *)&raddr, sizeof(raddr))<0)
    {
        perror("connetc()");
        exit(1);
    }

    // 3、将socket文件描述符转换为文件流，对文件流进行操作
    fp = fdopen(sd, "r+");
    if(fp == NULL)
    {
        perror("fdopen()");
        exit(1);
    }
    if(fscanf(fp, FMT_STAMP, &stamp)<1)
        fprintf(stderr, "Bad format!\n");
    else 
        fprintf(stdout, "stamp=%lld\n", stamp);

    fclose(fp);
    exit(1);
}