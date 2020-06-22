#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

static void cleanup(void *str)
{
    printf("cleanup: %s\n", (char *)str);
}


static void *thr_fn1(void *arg)
{
    printf("thread 1 start...\n");
    pthread_cleanup_push(cleanup, "thread 1 first handler");
    pthread_cleanup_push(cleanup, "thread 1 seconde handler");
    printf("thread 1 push complete\n");
    return (void *)1;
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
}

static void *thr_fn2(void *arg)
{
    printf("thread 2 start...\n");
    pthread_cleanup_push(cleanup, "thread 2 first handler");
    pthread_cleanup_push(cleanup, "thread 2 seconde handler");
    printf("thread 2 push complete\n");
    pthread_exit((void *)2);
    pthread_cleanup_pop(0);
    pthread_cleanup_pop(0);
}



int main()
{
    int err;
    pthread_t tid1, tid2;
    void *tret;

    err = pthread_create(&tid1, NULL, thr_fn1, (void *)1);
    if(err)
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1);
    }
    err = pthread_create(&tid2, NULL, thr_fn2, (void *)1);
    if(err)
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1);
    }

    pthread_join(tid1, &tret);
    printf("thread1 exit code %p\n", tret);
    pthread_join(tid2, &tret);
    printf("thread2 exit code %p\n", tret);

    exit(0);   
}