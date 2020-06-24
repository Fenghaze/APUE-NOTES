#include<stdio.h>
#include<stdlib.h>


int main()
{
    int fd[2];
    int ret = pipe(fd);
    if(ret == -1)
    {
        perror("pipe()");
        exit(1);
    }
    pid_t pid = fork();
    if(pid < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if(pid == 0)   // 子进程读
    {
        close(fd[1]);
        char buf[1024];
        ret = read(fd[0], buf, sizeof(buf));
        write(1, buf, ret);
        exit(0);
    }
    else
    {
        close(fd[0]);   
        write(fd[1], "hello", 5);
    }
    exit(0);   
}