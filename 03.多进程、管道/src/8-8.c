#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

int main()
{
    pid_t pid;
    if((pid = fork()) < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if(pid == 0)
    {
        if((pid = fork()) < 0)
        {
            perror("fork()");
            exit(1);
        }
        else if(pid > 0)
            exit(0);
        sleep(2);
        printf("[%d]2th child, ppid=%d\n", getpid(), getppid());
        exit(0);
    }
    if (waitpid(pid, NULL, 0)!=pid)
        perror("waitpid()"); 

    exit(0);
}