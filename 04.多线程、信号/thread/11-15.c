#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct msg
{
    struct msg *m_next;   /* data */
};

typedef struct msg msg;

msg *workq;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void process_msg()
{
    msg *mp;
    for (;; )
    {
        pthread_mutex_lock(&mut);
        while(workq == NULL)
            pthread_cond_wait(&cond, &mut);   /* code */
        mp = workq;
        workq = mp->m_next;
        pthread_mutex_unlock(&mut);
    }
    
}

void enqueue_msg(msg *mp)
{
    pthread_mutex_lock(&mut);
    mp->m_next = workq;
    workq = mp;
    pthread_mutex_unlock(&mut);
    pthread_cond_signal(&cond);
}