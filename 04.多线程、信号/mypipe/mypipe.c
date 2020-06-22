#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#include "mypipe.h"

struct mypipe_st
{
    int             head;           //管道头部
    int             tail;           //管道尾部
    char            data[PIPESIZE]; //管道长度
    int             datasize;       //管道的有效字符长度
    int             count_rd;   
    int             count_wr;
    pthread_mutex_t mut;            //操作管道的互斥量
    pthread_cond_t  cond;
};

mypipe_t *mypipe_init(void)
{
    struct mypipe_st *me;
    me = malloc(sizeof(*me));
    if(me == NULL)
        return;
    
    me->head = 0;
    me->tail = 0;
    me->datasize = 0;
    me->count_rd = 0;
    me->count_wr = 0;
    pthread_mutex_init(&me->mut, NULL);
    pthread_cond_init(&me->cond, NULL);
    return me;
}

int mypipe_destory(mypipe_t *ptr)
{
    struct mypipe_st *me = ptr;
    pthread_mutex_destroy(&me->mut);
    pthread_cond_destroy(&me->cond);
    free(ptr);
    return 0;
}

int mypipe_register(mypipe_t *ptr, int opmap)
{
    struct mypipe_st *me = ptr;
    pthread_mutex_lock(&me->mut);
    if(opmap & MYPIPE_READ)
        me->count_rd++;
    if(opmap & MYPIPE_WRITE)
        me->count_wr++;
    pthread_cond_broadcast(&me->cond);
    while (me->count_rd<=0 || me->count_wr<=0)
        pthread_cond_wait(&me->cond, &me->mut);
    
    pthread_mutex_unlock(&me->mut);    
    return 0;
}

int mypipe_unregister(mypipe_t *ptr, int opmap)
{
    struct mypipe_st *me = ptr;
    pthread_mutex_lock(&me->mut);
    if(opmap & MYPIPE_READ)
        me->count_rd--;
    if(opmap & MYPIPE_WRITE)
        me->count_wr--;
    pthread_cond_broadcast(&me->cond);

    pthread_mutex_unlock(&me->mut);    
    return 0;
}

static int mypipe_readbyte_unlocked(struct mypipe_st *me, char *datap)
{
    if(me->datasize <= 0)
        return -1;
    *datap = me->data[me->head];
    me->head = next(me->head);
    me->datasize--;
    return 0;
}

int mypipe_read(mypipe_t *ptr, void *buf, size_t count)
{
    struct mypipe_st *me = ptr;
    int i;
    pthread_mutex_lock(&me->mut);
    //当管道中没有字符时，等通知
    //解锁，让其他线程来向管道中写入字符
    while (me->datasize <= 0 && me->count_wr > 0)
        pthread_cond_wait(&me->cond, &me->mut);
    if(me->datasize <=0 && me->count_wr <=0)
    {
        pthread_mutex_unlock(&me->mut);
        return 0;
    }   
    for(i =0; i<count;i++)
    {
        if(mypipe_readbyte_unlocked(me, buf+i)<0)
            break;
    }
    // 读完之后，通知其他线程来写管道
    pthread_cond_broadcast(&me->cond);
    pthread_mutex_unlock(&me->mut);
    return i;
}

int mypipe_write(mypipe_t *, const void *buf, size_t count)
{
    
}