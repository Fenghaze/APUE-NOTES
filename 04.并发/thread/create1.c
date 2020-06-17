#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>
static void *func(void *p)
{
    puts("Thread is working...");
    pthread_exit(NULL); // 线程终止
}
int main()
{
    pthread_t tid;
    puts("Begin!");

    int err = pthread_create(&tid, NULL, func, NULL);
    if(err)
    {
        fprintf(stderr, "%s\n", strerror(err));
        exit(1);
    }
    pthread_join(tid, NULL);    // 线程收尸
    puts("End!");
    exit(0);

}