#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "proto.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <wait.h>
#define IPSTRSIZE 40
#define BUFSIZE 1024
#define PROCNUM 4 // 进程上限
static void server_job(int sd)
{
    char buf[BUFSIZE];
    int len;
    len = sprintf(buf, FMT_STAMP, (long long)time(NULL));
    // 发送一个时间辍给客户端
    if (send(sd, buf, len, 0) < 0)
    {
        perror("send()");
        exit(1);
    }
}

static void server_loop(int sd)
{
    socklen_t raddr_len;
    struct sockaddr_in raddr;
    int newsd;
    char ipstr[IPSTRSIZE];
    raddr_len = sizeof(raddr);
    while (1)
    {
        // 4、接受连接
        newsd = accept(sd, (void *)&raddr, &raddr_len);
        if (newsd < 0)
        {
            perror("accept()");
            exit(1);
        }
        inet_ntop(AF_INET, &raddr.sin_addr, ipstr, IPSTRSIZE);
        printf("[%d]Client:%s:%d\n", getpid(), ipstr, ntohs(raddr.sin_port));
        // 5、发送数据
        server_job(newsd);
        close(newsd);
    }
}

int main()
{
    int sd, i;
    pid_t pid;
    struct sockaddr_in laddr;

    // 1、取得socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("socket()");
        exit(1);
    }
    // 设置socket属性
    int val = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
    {
        perror("setsockopt()");
        exit(1);
    }
    // 2、bind
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(atoi(SERVERPORT));
    inet_pton(AF_INET, "0.0.0.0", &laddr.sin_addr);
    if (bind(sd, (void *)&laddr, sizeof(laddr)) < 0)
    {
        perror("bind()");
        exit(1);
    }

    // 3、监听模式
    if (listen(sd, 200) < 0)
    {
        perror("listen()");
        exit(1);
    }

    for (i = 0; i < PROCNUM; i++)
    {
        pid = fork();
        if (pid < 0)
        {
            perror("fork()");
            exit(1);
        }
        if (pid == 0)
        {
            server_loop(sd);
            exit(0);
        }
    }

    for (i = 0; i < PROCNUM; i++)
    {
        wait(NULL); /* code */
    }
    close(sd);
    exit(0);
}
