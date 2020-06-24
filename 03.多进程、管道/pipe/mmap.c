#include<stdlib.h>
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<sys/mman.h>
#include<string.h>
int main()
{
    int fd = open("/tmp/out", O_RDWR);
    if(fd < 0 )
    {
        perror("open()");
        exit(1);
    }  
    char *p = NULL;
    ftruncate(fd, 4);
    p = mmap(NULL, 4, PROT_READ|PROT_WRITE,\
                 MAP_SHARED, fd, 0);
    if(p == MAP_FAILED)
    {
        perror("mmap()");
        exit(1);
    }
    strcpy(p, "abc");   // 向映射区内写数据
    close(fd);
    int ret = munmap(p, 4); // 关闭映射区
    if(ret < 0)
    {
        perror("munmap()");
        exit(1);
    }
    exit(0);
}