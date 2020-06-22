#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#define LEFT  30000000
#define RIGHT 30000200

/*利用父子进程找质数，缩短执行时间*/
int main()
{
    int mark;
    for (int i = LEFT; i <= RIGHT; i++)
    {
        pid_t pid;
        // 父进程负责 fork 200次
        pid = fork();
        if (pid < 0)
        {
            perror("fork()");
            exit(1);   /* code */
        }
        if(pid == 0) //子进程负责找质数
        {
            mark = 1;
            for (int j = 2; j < i/2; j++)
            {
                if (i%j ==0)
                {
                    mark=0;
                    break;   /* code */
                }
                
            }
            if (mark)
            {
                printf("%d is primer\n", i);/* code */
            }
            exit(0); // 子进程结束!!!
        }        
    }
    //int st;
    for (int i = LEFT; i <= RIGHT; i++)
    {
     //   wait(&st);   /* code */
        wait(NULL); // 收尸
    }
    exit(0);

}


