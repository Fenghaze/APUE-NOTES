#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define SERV_PORT 6666
#define IPSIZE 40
int main()
{
    int lfd, cfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t len;
    char buf[BUFSIZ];
    int n;
    char client_IP[IPSIZE];
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd < 0)
    {
        perror("socket()");
        exit(1);
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr);

    if(bind(lfd, (void *)&serv_addr, sizeof(serv_addr))<0)
    {
        perror("bind()");
        exit(1);
    }

    if(listen(lfd, 128)<0)
    {
        perror("listen()");
        exit(1);
    }

    len = sizeof(cli_addr);
    cfd = accept(lfd, (void *)&cli_addr, &len);
    if(cfd < 0)
    {
        perror("accept()");
        exit(1);
    }
    inet_ntop(AF_INET, &cli_addr.sin_addr, client_IP, IPSIZE);
    printf("client IP:%s:%d\n", client_IP, ntohs(cli_addr.sin_port));
    while (1)
    {
        // 读取客户端的socket
        n = read(cfd, buf, BUFSIZ);
        // 将读取到的数据进行转换
        for (int i = 0; i < n; i++)
        {
            buf[i] = toupper(buf[i]); /* code */
        }
        // 将转换后的结果写入客户端的socket
        write(cfd, buf, n);
    }
    close(lfd);
    close(cfd);
    return 0;
}
