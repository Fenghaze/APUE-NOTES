/*使用封装好的令牌桶函数库来实现cat命令，每秒打印CPS个字符*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "mytbf.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<string.h>
#define CPS 10
#define BUFSIZE 1024
#define BURST 100

int main(int argc, char **argv)
{
    int sfd, dfd = 1;
    char buf[BUFSIZE];
    int len, ret, pos;
    mytbf_t *tbf;
    int size = 0;
    if (argc < 2)
    {
        fprintf(stderr, "Usage...\n");
        exit(1); /* code */
    }

    tbf = mytbf_init(CPS, BURST);
    if (tbf == NULL)
    {
        fprintf(stderr, "Init Error");
        exit(1); /* code */
    }

    do
    {
        sfd = open(argv[1], O_RDONLY); /* code */
        if (sdf < 0)
        {
            if (errno != EINTR) // EINTR是信号发出的中断错误，假错，应该忽略
            {
                perror("open()");
                exit(1);
            } /* code */
        }
    } while (sdf < 0);
    while (1)
    {
        size = mytbf_fetchtoken(tbf, BUFSIZE);
        if (size < 0)
        {
            fprintf(stderr, "mytnf_fetchtoken %s\n", strerror(-size)); /* code */
            exit(1);
        }

        while ((len = read(sfd, buf, size)) < 0)
        {
            if (errno == EINTR)
                continue;
            perror("read()");
            break;
        }
        if (len == 0)
            break;
        if (size - len > 0)
        {
            mytbf_returntoken(tbf, size - len); /* code */
        }

        pos = 0;
        while (len > 0)
        {
            ret = write(dfd, buf + pos, len);
            if (ret < 0)
            {
                if (errno == EINTR)
                    continue;
                perror("write()");
                exit(1); /* code */
            }
            pos += ret;
            len -= ret;
            /* code */
        }
    }
    close(sfd);
    mytbf_destory(tbf);
    exit(0);
}