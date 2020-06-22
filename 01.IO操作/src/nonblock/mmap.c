#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
int main(int argc, char *argv[])
{

    int fd;
    struct stat res;
    char *str;
    int count = 0;
    fd = open(argv[1], O_RDONLY);

    fstat(fd, &res);

    str = mmap(NULL, res.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (str == MAP_FAILED)
    {
        perror("mmap()");
        exit(1);
    }
    close(fd);
    for (int i = 0; i < res.st_size; i++)
    {
        if (str[i] == 'a')
            count++; /* code */
    }
    printf("%d\n", count);

    munmap(str, res.st_size);  
    exit(0);
}