#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include "proto.h"
#include <arpa/inet.h>

#define IPSTRSIZE   40
#define BUFSIZE     1024
static void server_job(int sd)
{
    char buf[BUFSIZE];
    int len;
    len = sprintf(buf, FMT_STAMP, (long long)time(NULL));
    // 发送一个时间辍给客户端
    if(send(sd, buf, len, 0)<0)
    {
        perror("send()");
        exit(1);
    }
}


int main()
{
    int sd, newsd;
    struct sockaddr_in laddr, raddr;
    socklen_t raddr_len;
    char ipstr[IPSTRSIZE];
    // 1、取得socket    
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0)
    {
        perror("socket()");
        exit(1);
    }
    // 设置socket属性
    int val =1 ;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val))<0)
    {
        perror("setsockopt()");
        exit(1);
    }
    // 2、bind
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
    if(bind(sd, (void *)&laddr, sizeof(laddr))<0)
    {
        perror("bind()");
        exit(1);
    }

    // 3、监听模式
    if(listen(sd, 200) < 0)
    {
        perror("listen()");
        exit(1);
    }
   
    raddr_len = sizeof(raddr);
    while(1)
    { 
        // 4、接受连接
        newsd = accept(sd, (void *)&raddr, &raddr_len);
        if(newsd < 0)
        {
            perror("accept()");
            exit(1);
        }
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
        printf("Client:%s:%d\n", ipstr, ntohs(raddr.sin_port));
        // 5、发送数据
        server_job(newsd);
        close(newsd);
    }
    // 6、关闭socket
    close(sd);
    exit(1);
}
