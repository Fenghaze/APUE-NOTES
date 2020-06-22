#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define NHASH 29
#define HASH(id) (((unsigned long)id) % NHASH)

struct foo
{
    int f_count;
    pthread_mutex_t f_lock;
    int f_id;
    struct foo *f_next;
};

typedef struct foo foo;
foo *fh[NHASH];
pthread_mutex_t hashlock = PTHREAD_MUTEX_INITIALIZER;

foo *foo_alloc(int id)
{
    foo *fp;
    int idx;
    if ((fp = malloc(sizeof(foo))) != NULL)
    {
        fp->f_count = 1;
        fp->f_id = id;
        if (pthread_mutex_init(&fp->f_lock, NULL) != 0)
        {
            free(fp);
            return NULL;
        }
        idx = HASH(id);
        pthread_mutex_lock(&hashlock, NULL);
        fp->f_next = fh[idx];
        fh[idx] = fp;
        pthread_mutex_lock(&fp->f_lock);
        pthread_mutex_unlock(&hashlock);
        pthread_mutex_unlock(&fp->f_lock);
    }
    return fp;
}

void foo_hold(foo *fp)
{
    pthread_mutex_lock(&fp->f_lock);
    fp->f_count++;
    pthread_mutex_unlock(&fp->f_lock);
}

foo *foo_find(int id)
{
    foo *fp;
    pthread_mutex_lock(&hashlock);
    for (fp = fh[HASH(id)]; fp != NULL; fp = fp->f_next)
    {
        if (fp->f_id == id)
        {
            foo_hold(fp);
            break;
        }
    }
    pthread_mutex_unlock(&hashlock);
    return fp;
}
void free_rele(foo *fp)
{
    foo *tfp;
    int idx;

    pthread_mutex_lock(&fp->f_lock);
    if (--fp->fcount == 0)
    {
        idx = HASH(fp->f_id);
        tfp = fh[idx];
        if (tfp == fp)
        {
            fh[idx] = fp->f_next; 
        }
        else
        {
            while (tfp->f_next != fp)
            {
                tfp = tfp->f_next; 
            }
            tfp->f_next = fp->f_next;
        }
        pthread_mutex_unlock(&hashlock);
        pthread_mutex_destroy(&fp->f_lock);
        free(fp);
    }
    else
    {
        pthread_mutex_unlock(&hashlock);
    }
}
