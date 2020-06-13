#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define LEFT 30000000
#define RIGHT 30000200

/*利用父子进程找质数，缩短执行时间*/
int main()
{
    int mark;
    int n;
    pid_t pid;
    int N = 3;
    for (n = 0; n < N; n++)
    {
        pid = fork(); /* code */
        if (pid < 0)
        {
            perror("fork()");
            exit(1);    /* code */
        }
        
        if (pid == 0)
        {
            for (int i = LEFT+n; i <= RIGHT; i+=N)
            {
                mark = 1;
                for (int j = 2; j < i / 2; j++)
                {
                    if (i % j == 0)
                    {
                        mark = 0;
                        break; /* code */
                    }
                }
                if (mark)
                {
                    printf("[%d]:%d is primer\n", n,i); /* code */
                }
            }            
            exit(0); // 子进程结束!!!
        }
    }

    for (n = 0; n <N; n++)
    {
        wait(NULL); // 收尸
    }
    exit(0);
}
