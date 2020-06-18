#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
#define THRNUM      4

static pthread_mutex_t mut[THRNUM];

static int next(int n)
{
    if(n+1 == THRNUM)
        return 0;
    return n+1;
}

static void *thr_func(void *p)
{

    int n = int(p);
    int c = 'a' + n;

    while (1)
    {
        pthread_mutex_lock(mut+n); // main线程for循环结束前，每把锁阻塞
        write(1, &c ,1);   // for循环结束后，第一把锁解开，a写入，并解锁下一把锁
        pthread_mutex_unlock(mut+next(n));
    }
    pthread_exit(NULL);
}


int main()
{
    pthread_t tid[THRNUM];
    int i,err;
    for ( i = 0; i < THRNUM; i++)
    {
        pthread_mutex_init(mut+i, NULL); //初始化锁
        pthread_mutex_lock(mut+i); //每把锁都上锁
        err = pthread_create(tid+i, NULL, thr_func, (void *)i);   /* code */
        if(err)
        {
            fprintf(stderr, "%s\n", strerror(err));
        }
    }
    pthread_mutex_unlock(mut+0);    //解第一把锁
    alarm(5);
    for ( i = 0; i < THRNUM; i++)
    {
        pthread_join(tid[i], NULL);   /* code */
    }
    exit(0);    
}