#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
 #include <arpa/inet.h>
#define SERV_PORT 6666

int main()
{
    int cfd;
    struct sockaddr_in serv_addr;
    socklen_t len;
    char buf[BUFSIZ];
    int n;
    cfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    if(connect(cfd, (void *)&serv_addr, sizeof(serv_addr))<0)
    {
        perror("connect()");
        exit(1);
    }
    
    while (1)
    {
        fgets(buf, sizeof(buf), stdin);
        // 向当前客户端的socket写入数据
        write(cfd, buf, strlen(buf));
        // 服务端对写入的数据进行转化，将结果写入到客户端的socket
        // 再从客户端的socket读取数据
        n = read(cfd, buf, sizeof(buf));
        write(STDOUT_FILENO, buf, n);
    }
    close(cfd);
    return 0;
}
