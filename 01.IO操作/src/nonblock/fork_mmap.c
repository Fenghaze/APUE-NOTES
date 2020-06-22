#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<sys/wait.h>
#define MEMSIZE     1024

int main()
{
    pid_t pid;
    char *ptr;

    ptr = mmap(NULL, MEMSIZE, O_RDWR, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    if((pid=fork()) < 0)
    {
        perror("fork()");
        munmap(ptr, MEMSIZE);
        exit(1);
    }
    if(pid == 0)
    {
        strcpy(ptr, "Helo");
        munmap(ptr, MEMSIZE);
        exit(0);
    }
    else
    {
        wait(NULL);
        puts(ptr);
        munmap(ptr, MEMSIZE);
        exit(0);   
    }
}