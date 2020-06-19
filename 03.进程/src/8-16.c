#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main()
{
    pid_t pid;
    if ((pid = fork()) < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if (pid == 0)
    {
        execl("/bin/echo", "echo", "hello world", NULL);
        perror("execle()");
        exit(1);
    }
    wait(NULL);

    exit(0);
}