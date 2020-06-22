#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#define THRNUM      4

static int num = 0;
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


static int next(int n)
{
    if(n+1 == THRNUM)
        return 0;
    return n+1;
}

static void *thr_func(void *p)
{

    int n = (int)p;
    int c = 'a' + n;

    while (1)
    {
        pthread_mutex_lock(&mut); // main线程for循环结束前，加锁阻塞
        while (num!=n)
        {
            pthread_cond_wait(&cond, &mut);   /* code */
        }
        
        write(1, &c ,1);   // for循环结束后，第一把锁解开，a写入，并解锁下一把锁
        num = next(num);
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mut);
    }
    pthread_exit(NULL);
}


int main()
{
    pthread_t tid[THRNUM];
    int i,err;
    for ( i = 0; i < THRNUM; i++)
    {
        err = pthread_create(tid+i, NULL, thr_func, (void *)i);   /* code */
        if(err)
        {
            fprintf(stderr, "%s\n", strerror(err));
        }
    }
    alarm(5);
    for ( i = 0; i < THRNUM; i++)
    {
        pthread_join(tid[i], NULL);   /* code */
    }

    pthread_mutex_destroy(&mut);
    pthread_cond_destroy(&cond);

    exit(0);    
}

/*代码解析：
main中的for循环：
创建4个线程ABCD，4个线程(thr_func)同时运行



thr_func的while循环：ABCD去抢一个锁

假设B抢到锁，进行加锁
发现num！=n，解锁，等待通知，直到num=n才跳出循环（num=0，n=1）
因为解锁了，其他线程又可以抢锁了

假设C抢到锁，进行加锁
发现num！=n，解锁，等待通知...（num=0，n=1）
其他线程又抢锁

假设A抢到锁，进行加锁
发现num==n（num=0，n=0）
开始写入操作（打印a），并改变num值
通知其他所有线程可以来抢锁，并解锁

假设D抢到锁，进行加锁
发现num！=n，解锁，等待通知...（num=1，n=3）
其他线程又抢锁

假设B抢到锁，进行加锁
发现num==n（num=1，n=1）
开始写入操作（打印b），并改变num值
通知其他所有线程可以来抢锁，并解锁

.......
*/