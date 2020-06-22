#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFSIZE 1024
int main()
{
    pid_t pid;
    int pd[2];
    char buf[BUFSIZE];
    int len;
    if (pipe(pd) < 0)
    {
        perror("pipe()");
        exit(1);
    }

    if ((pid = fork()) < 0)
    {
        perror("fork()");
        exit(1);
    }
    if (pid == 0)
    {
        close(pd[1]); //关闭写管道
        len = read(pd[0], buf, BUFSIZE);
        write(1, buf, len);
        close(pd[0]); //关闭读管道
        exit(0);
    }
    else
    {
        close(pd[0]);
        write(pd[1], "hello", 5);
        close(pd[1]);
        wait(NULL);
        exit(0);
    }
}