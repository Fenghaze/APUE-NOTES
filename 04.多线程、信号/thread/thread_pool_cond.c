#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#define LEFT 30000000
#define RIGHT 30000200
#define THRNUM 4

static int num = 0;
static pthread_mutex_t mut_num = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond_num = PTHREAD_COND_INITIALIZER;
/*
任务池：main线程发放任务
使用num来标记是否有任务
num>0(LEFT-RIGHT)：当前有任务，线程可以来抢；
num=0：当某个线程抢到任务后，num=0，main线程继续发放任务；
num<0：所有质数发放完毕，结束

使用通知法来替换查询法（盲等）
*/


static void *thr_prime(void *p)
{
    int i, mark;
    // 线程一直抢任务，用while循环
    while (1)
    {
        pthread_mutex_lock(&mut_num); // 加锁，对num进行操作
        while (num == 0) // 如果num=0，说明没有任务，需要main线程发放任务
        {   // 解锁，等待main线程num改变的通知
            pthread_cond_wait(&cond_num, &mut_num);
        }
        if (num == -1) // 如果num=-1，没有任务了，就不计算
        {
            pthread_mutex_unlock(&mut_num); // 跳出时，一定要解锁
            break;
        }
        // 在锁的阶段，对num进行操作
        i = num;    
        num = 0; // 接收到任务后，改变num=0，解锁（通知main线程可以抢锁来发放任务）
        pthread_cond_broadcast(&cond_num); //通知所有线程来抢锁，对num进行操作
        pthread_mutex_unlock(&mut_num); //解锁
        mark = 1;
        for (int j = 2; j < i / 2; j++)
        {
            if (i % j == 0)
            {
                mark = 0;
                break;
            }
        }
        if (mark)
            printf("[%d:]%d is primer\n",(int)p, i); /* code */
    }
    pthread_exit(NULL); // 线程终止，并返回一个值
}

int main()
{
    int err;
    pthread_t tid[THRNUM];
    int i;
    for (i = 0; i < THRNUM; i++)
    {
        // 创建 4 个线程来抢任务
        err = pthread_create(tid + i, NULL, thr_prime, (void *)i);
        if (err)
        {
            fprintf(stderr, "%s\n", strerror(err));
            exit(1);
        }
    }
    // 发放任务
    for (i = LEFT; i < RIGHT; i++)
    {
        pthread_mutex_lock(&mut_num);   // 加锁，来对num进行修改
        while (num != 0) // 如果num！=0,就等待num变成0
        {   // 解锁mut_num，等待一个num=0的通知
            pthread_cond_wait(&cond_num, &mut_num);
        }
        num = i; // while退出，num=0时，才发放任务
        pthread_cond_signal(&cond_num); //num改变了，通知任意一个线程来抢锁
        pthread_mutex_unlock(&mut_num); // 解锁，让其他线程来抢锁执行任务
    }

    pthread_mutex_lock(&mut_num); // 对num进行操作，加锁
    while(num!=0)   // 需要确保所有任务都执行完毕
    {   // 解锁，等待一个num=0的通知
        pthread_cond_wait(&cond_num, &mut_num);
    }
    num = -1; // 任务发放完毕，没有任务了，num=-1
    pthread_cond_broadcast(&cond_num);  // num改变了，通知所有线程来抢锁
    pthread_mutex_unlock(&mut_num); //解锁
    for (i = 0; i < THRNUM; i++)
    {
        // 将所有线程收尸
        pthread_join(tid[i], NULL);
    }
    pthread_mutex_destroy(&mut_num);
    pthread_cond_destroy(&cond_num);
    exit(0);
}
