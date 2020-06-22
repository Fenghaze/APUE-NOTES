#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#define PROCNUM 20
#define FNAME "/tmp/out"
#define LINESIZE 1024
static void func_add()
{
    // 每个子进程读写同一个文件
    FILE *fp = fopen(FNAME, "r+");
    char linebuf[LINESIZE];
    if (!fp)
    {
        perror("fopen()");
        exit(1);
    }
    int fd = fileno(fp);

    lockf(fd, F_LOCK, 0);
    fgets(linebuf, LINESIZE, fp);
    fseek(fp, 0, SEEK_SET); //定位
    fprintf(fp, "%d\n", atoi(linebuf) + 1);
    fflush(fp);
    lockf(fd, F_ULOCK, 0);
    fclose(fp);
    return;
}

int main()
{
    int i, err;
    pid_t pid;
    for (i = 0; i < PROCNUM; i++)
    {
        if ((pid = fork()) < 0)
        {
            perror("fork()");
            exit(1);
        }
        if (pid == 0)
        {
            func_add();
            exit(0);
        }
    }

    for (i = 0; i < PROCNUM; i++)
    {
        wait(NULL); /* code */
    }

    exit(0);
}