#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#define MYRTSIG     (SIGRTMIN+6)
static void mysig_handler(int s)
{
    write(1, "!", 1);
}

int main()
{
    sigset_t set, oset;
    signal(MYRTSIG, mysig_handler);
    sigemptyset(&set); //初始化信号集合
    sigaddset(&set, MYRTSIG); //添加SIGINT信号
    sigprocmask(SIG_BLOCK, &set, &oset); //屏蔽信号
    for (int j = 0; j < 1000; j++)
    {
        for (int i = 0; i < 3; i++)
        {
            write(1, "*", 1);
            sleep(1); /* code */
        }
        write(1, "\n", 1);
        sigsuspend(&oset);
    }
    exit(0);
}