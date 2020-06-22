#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>
void *func(void *p)
{
    while(1)
        pause();
    pthread_exit(NULL);
}

int main()
{
    int i, err;
    pthread_t tid;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 1024*1024);
    for ( i = 0;; i++)
    {
        err = pthread_create(&tid, &attr, func, NULL);   /* code */
        if (err)
        {
            fprintf(stderr, "%s\n", strerror(err));
            exit(1);   /* code */
        }
    }
    printf("%d\n", i);
    pthread_attr_destroy(&attr);
    exit(0);
}