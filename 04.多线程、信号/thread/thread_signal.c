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
#define N    4  //同时运行的线程个数上限

struct mysem_st
{
    int value;
    pthread_mutex_t mut;
    pthread_cond_t cond;   /* data */
};
typedef struct mysem_st mysem_st;

mysem_st *sem;

mysem_st *mysem_init(int initval) // 初始化信号量
{
    struct mysem_st *me;
    me = malloc(sizeof(*me));
    if (me == NULL)
    {
        return NULL;
    }
    me->value = initval;    // 信号量个数
    pthread_mutex_init(&me->mut, NULL); // 初始化互斥量
    pthread_cond_init(&me->cond, NULL); // 初始化条件变量
    return me;
}

int mysem_sub(mysem_st *ptr, int n)
{
    struct mysem_st *me = ptr;
    // 对信号量的操作进行加锁
    pthread_mutex_lock(&me->mut);
    while (me->value < n) // 如果没有信号量了，就等待信号量的归还
    {
        // 解锁，等待信号量改变的通知
        pthread_cond_wait(&me->cond, &me->mut);   /* code */
    }
    // 有信号量了，就减去消耗的信号量个数
    me->value -= n;
    // 解锁
    pthread_mutex_unlock(&me->mut);
    return n;
}

int mysem_add(mysem_st *ptr, int n)
{
    struct mysem_st *me = ptr;
    // 对信号量的操作进行加锁
    pthread_mutex_lock(&me->mut);
    // 加上归还的信号量
    me->value += n;
    // 通知所有线程，信号量有变动
    pthread_cond_broadcast(&me->cond);
    // 解锁
    pthread_mutex_unlock(&me->mut);
    return n;
}

int mysem_destory(mysem_st *ptr) // 摧毁结构体
{
    struct mysem_st *me = ptr;
    pthread_mutex_destroy(&me->mut); // 摧毁互斥量
    pthread_cond_destroy(&me->cond); // 摧毁条件变量
    free(me);
    return 0;
}

static void *thr_prime(void *p) // 每个线程算一个数
{
    int i = (int)p;
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
//    sleep(2);
    mysem_add(sem, 1); // 归还信号量
    pthread_exit(NULL); // 线程终止，并返回一个值
}

int main()
{
    int err;
    pthread_t tid[THRNUM];
    int i;
    sem = mysem_init(N); // 初始化信号量，只允许N个线程访问资源
    if(sem == NULL)
    {
        fprintf(stderr, "init failed!\n");
        exit(1);
    }

    for (i = LEFT; i <= RIGHT; i++) //创建200个线程
    {
        // 但只允许 4 个线程来抢资源
        mysem_sub(sem, 1);   // 可共享资源的线程个数 -1
        err = pthread_create(tid + (i-LEFT), NULL, thr_prime, (void *)i);
        if (err)
        {
            fprintf(stderr, "%s\n", strerror(err));
            exit(1);
        }
    }
    for (i = LEFT; i <= RIGHT; i++)
    {
        // 将所有线程收尸，ptr接收线程终止返回的值
        pthread_join(tid[i-LEFT], NULL);
    }

    mysem_destory(sem);

    exit(0);
}
