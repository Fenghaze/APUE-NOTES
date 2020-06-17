#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM (RIGHT - LEFT + 1)

struct thr_arg_st
{
    int n;
};

static void *thr_prime(void *p)
{
    int i;
    i = ((struct thr_arg_st *)p)->n;
    int mark = 1;
    for (int j = 2; j < i / 2; j++)
    {
        if (i % j == 0)
        {
            mark = 0;
            break; /* code */
        }
    }
    if (mark)
        printf("%d is primer\n", i); /* code */
    pthread_exit(p); // 线程终止，并返回一个值
}

int main()
{
    int err;
    pthread_t tid[THRNUM];
    int i;
    struct thr_arg_st *p;
    void* ptr;
    for (i = LEFT; i <= RIGHT; i++)
    {
        p = malloc(sizeof(*p));
        if( p== NULL)
        {
            perror("malloc()");
            exit(1);
        }
        p->n = i;
        // 创建 201 个线程来分别计算每个数是否为质数
        err = pthread_create(tid + i - LEFT, NULL, thr_prime, p);
        if (err)
        {
            fprintf(stderr, "%s\n", strerror(err));
            exit(1);
        }
    }
    for (i = LEFT; i <= RIGHT; i++)
    {
        // 将所有线程收尸，ptr接收线程终止返回的值
        pthread_join(tid[i - LEFT], &ptr);
        free(ptr); /* code */
    }
    exit(0);
}
