#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>

static void charatatime(char *str)
{
    char *ptr;
    int c;
    setbuf(stdout, NULL);
    for ( ptr = str; (c=*ptr++) != 0; )
        putc(c, stdout); 
}

int main()
{
    pid_t pid;
    if((pid=fork()) < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if(pid == 0)
    {
        charatatime("output from child\n");
    }
    else
    {
        charatatime("output from parrent\n");         
    }
    exit(0);
}