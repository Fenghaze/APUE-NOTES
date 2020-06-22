#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<unistd.h>

char buf[] = "hello wrold\n";
int val = 6;

int main()
{
    pid_t pid;
    int var = 80;
    if(write(1, buf, sizeof(buf)-1) != sizeof(buf)-1)
    {
        perror("write()");
        exit(1);
    }
    printf("[%ld]Begin Fork!\n", (long)getpid());
    fflush(NULL);
    if((pid = fork()) < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if(pid == 0)
    {
        val++;
        var++;
    }
    else
    {
        val += 2;
        var += 2;    
        sleep(2);
    }

    printf("pid=%ld, val=%d, var=%d, &val=%p, &var=%p\n", (long)getpid(), val, var, &val, &var);
    
    exit(0);    
}