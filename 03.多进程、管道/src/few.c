#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

int main()
{
    puts("Begin!");
    fflush(NULL);
    int pid = fork();

    if (pid < 0)
    {
        perror("fork()");
        exit(1);    /* code */
    }
    if(pid == 0)
    {
        execl("/bin/date", "date", "+%s", NULL);
        perror("execl()");
        exit(1);
    }
    wait(NULL);

    puts("End!");
    exit(0);
}