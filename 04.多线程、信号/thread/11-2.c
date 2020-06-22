#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
pthread_t ntid;

static void printtids(const char *s)
{
    pid_t pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();
    printf("%s pid %lu tid %lu (0x%lx)\n", s, (unsigned long)pid,
    (unsigned long)tid, (unsigned long)tid);
}

static void *thr_fn(void *arg)
{
    printtids("new thread: ");
    pthread_exit(NULL);
}

int main()
{
    int err;
    err = pthread_create(&ntid, NULL, thr_fn, NULL);
    if (err != 0)
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1);   /* code */
    }
    printtids("main thread: ");    
    sleep(1);
    pthread_join(ntid, NULL);

    exit(0);
}