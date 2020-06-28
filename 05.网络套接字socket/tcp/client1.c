#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>

#define SERV_PORT   6666    

int main()
{
    int lfd, cfd;
    struct sockaddr_in serv_addr, cli_addr;    
    socklen_t len;
    char buf[BUFSIZ];
    int n;
    lfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(lfd, "0.0.0.0", &serv_addr.sin_addr);

    bind(lfd, (void *)&serv_addr, sizeof(serv_addr));

    listen(lfd, 128);
    
    len = sizeof(cli_addr);
    cfd = accept(lfd, (void *)&cli_addr, &len);

    n = read(cfd, buf, BUFSIZ);
    for (int i = 0; i < n; i++)
    {
        buf[i] = toupper(buf[i]);   /* code */
    }
    write(cfd, buf, n);

    close(lfd);
    close(cfd);
    return 0;
}
