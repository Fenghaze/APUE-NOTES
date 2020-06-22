#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include "apue.h"

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void prepare()
{
    int err;
    printf("preaparing locks...\n");
    if((err=pthread_mutex_lock(&lock1)!=0))
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
    if((err=pthread_mutex_lock(&lock2)!=0))
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
}

void parent()
{
    int err;
    printf("parent unlocking locks...\n");
    if((err=pthread_mutex_unlock(&lock1)!=0))
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
    if((err=pthread_mutex_unlock(&lock2)!=0))
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
}

void child()
{
    int err;
    printf("child unlocking locks...\n");
    if((err=pthread_mutex_unlock(&lock1)!=0))
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
    if((err=pthread_mutex_unlock(&lock2)!=0))
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
}

void *thr_fn(void *arg)
{
    printf("thread started...\n");
    pause();
    return 0;
}

int main()
{
    int err;
    pid_t pid;
    pthread_t tid;

    if ((err = pthread_atfork(prepare, parent, child)) != 0)
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
    }
    if ((err = pthread_create(&tid, NULL, thr_fn, 0)) != 0)
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1); /* code */
        /* code */
    }
    sleep(2);
    printf("parent about to fork...\n");
    if ((pid = fork()) < 0)
    {
        perror("fork()");
        exit(1);
    }
    else if (pid == 0)
        printf("child returned from fork\n");
    else
        printf("partent returned from fork\n");

    exit(0);
}
