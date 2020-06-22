#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static void int_handler(int s)
{
    write(1, "!", 1);
}

int main()
{
    sigset_t set;
    signal(SIGINT, int_handler);
    sigemptyset(&set); //初始化信号集合
    sigaddset(&set, SIGINT); //添加SIGINT信号
    for (int j = 0; j < 1000; j++)
    {
        sigprocmask(SIG_BLOCK, &set, NULL); //屏蔽信号
        for (int i = 0; i < 10; i++)
        {
            write(1, "*", 1);
            sleep(1); /* code */
        }
        write(1, "\n", 1);
        sigprocmask(SIG_UNBLOCK, &set, NULL);//恢复信号
    }
    exit(0);
}