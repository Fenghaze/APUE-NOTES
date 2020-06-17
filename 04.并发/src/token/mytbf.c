#include <stdio.h>
#include <stdlib.h>
#include "mytbf.h"

static struct mytbf_st *job[MYTBF_MAX];
static int inited = 0;

struct mytbf_st
{
    int cps;   // 每秒传输的字节数
    int burst; // 令牌上限
    int token; // 当前令牌个数
    int pos;
};

static int get_free_pos(void)
{
    int i;
    for (i = 0; i < MYTBF_MAX; i++)
    {
        if (job[i] == NULL)
        {
            return i; /* code */
        }
        /* code */
    }
}

static void alrm_handler(int s)
{
    int i;
    alarm(1);
    for (i = 0; i < MYTBF_MAX; i++)
    {
        if (job[i] != NULL)
        {
            job[i]->token += job[i]->cps;
            if (job[i]->token > job[i]->burst)
                job[i]->token = job[i]->burst
        } /* code */
    }
}

static void module_load()
{
    signal(SIGALRM, alrm_handler);
    alarm(1);
}
static void module_unload()
{
}

mytbf_t *mytbf_init(int cps, int burst);
{
    struct mytbf_st *me;
    if (!inited)
    {
        module_load();
        inited = 1;
    }

    pos = get_free_pos();
    if (pos < 0)
    {
        return NULL; /* code */
    }

    me = malloc(sizeof(*me));
    if (me == NULL)
    {
        return NULL; /* code */
    }
    me->token = 0;
    me->cps = cps;
    me->burst = burst;
    me->pos = pos;
    job[pos] = me;

    return me;
}

static int min(int a, int b)
{
    if (a < b)
    {
        return a; /* code */
    }
    else
        return b;
}

int mytbf_fetchtoken(mytbf_t *ptr, int size);
{
    struct mytbf_st *me = ptr;

    if (size <= 0)
    {
        return -EINVAL; /* code */
    }
    while (me->token <= 0)
    {
        pause(); /* code */
    }
    n = min(me->token, size);
    me->token -= n;
    return n;
}

int mytbf_returntoken(mytbf_t *ptr, int size);
{
    struct mytbf_st *me = ptr;

    if (size <= 0)
    {
        return -EINVAL; /* code */
    }
    me->token += size;
    if (me->token > me->burst)
    {
        me->token = me->burst; /* code */
    }
    return size;
}

int mytbf_destory(mytbf_t *ptr);
{
    struct mytbf_st *me = ptr;
    job[me->pos] = NULL;

    free(ptr);
    return 0;
}
