#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

struct job
{
    struct job  *j_next;
    struct job  *j_prev;
    pthread_t   j_id;   //用于保护这个对象的互斥量
}

struct queue
{
    struct job          *q_head;
    struct job          *q_tail;
    pthread_rwlock_t     q_lock;     //用于保护这个队列的读写锁
};

typedef struct job      job;
typedef struct queue    queue;

int queue_init(queue *qp)
{
    int err;
    qp->q_head = NULL;
    qp->q_tail = NULL;
    err = pthread_rwlock_init(&qp->q_lock, NULL);
    if (err != 0)
        return err;
    return 0;
}

void job_insert(queue *qp, job *jp)
{
    pthread_rwlock_wrlock(&qp->q_lock);
    jp->j_next = qp->q_head;
    jp->j_prev = NULL;
    if (qp->q_head != NULL)
    {
        qp->q_head->j_prev = jp;   /* code */
    }
    else
    {
        qp->q_tail = jp;
    }
    qp->q_head = jp;
    pthread_rwlock_unlock(&qp->q_lock);
}

job *job_find(queue *qp, pthread_t id)
{
    job *jp;
    if(pthread_rwlock_rdlock(&qp->q_lock) != 0)
        return NULL;
    for (jp=qp->q_head; jp!=NULL; jp=jp->j_next)
    {
        if (pthread_equal(jp->j_id, id))
        {
            break;   /* code */
        }
    }
    pthread_rwlock_unlock(&qp->q_lock);
    return jp; 
}