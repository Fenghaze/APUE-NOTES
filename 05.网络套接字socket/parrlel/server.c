#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVERPORT 8888

void wait_child(int signo)
{
    while (waitpid(0, NULL, WNOHANG) > 0)
        ;
    return;
}

int main()
{
    int lfd, cfd;
    struct sockaddr_in serv_addr, clie_addr;
    socklen_t len;
    pid_t pid;
    int n;
    char buf[BUFSIZ], clie_ip[40];

    lfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);
    inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr);

    bind(lfd, (void *)&serv_addr, sizeof(serv_addr));

    listen(lfd, 128);

    while (1)
    {
        len = sizeof(clie_addr);

        cfd = accept(lfd, (void *)&clie_addr, &len);
        inet_ntop(AF_INET, &clie_addr.sin_addr, &clie_ip, sizeof(clie_ip));
        printf("client IP:%s:%d\n", &clie_ip, ntohs(clie_addr.sin_port));
        if ((pid = fork()) < 0)
        {
            perror("fork()");
            exit(1);
        }
        else if (pid == 0)
        {
            close(lfd);
            break;
        }
        else
        {
            close(cfd);
            // 收尸
            signal(SIGCHLD, wait_child);
        }
    }
    if (pid == 0)
    {
        while (1)
        {
            n = read(cfd, buf, BUFSIZ);
            if (n == 0)
            {
                close(cfd);
                return 0;
            }
            else if (n == -1)
            {
                perror("read()");
                exit(1);
            }
            else
            {
                for (int i = 0; i < n; i++)
                {
                    buf[i] = toupper(buf[i]); /* code */
                }
                write(cfd, buf, n);
            }
        }
    }

    return 0;
}
