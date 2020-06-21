#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include "realyer.h"
enum
{
    STATE_R=1,
    STATE_W,
    STATE_Ex,
    STATE_T
};

struct rel_fsm_st
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *errstr;
    int64_t count;
};

struct rel_job_st
{
    int job_state;
    int fd1;
    int fd2;
    struct rel_fsm_st fsm12, fsm21;
    int fd1_save, fd2_save;
    //struct timerval start, end;
}

static struct rel_job_st *rel_job[REL_JOBMAX];

static void fsm_driver(struct fsm_st *fsm)
{
    int ret;   
    switch (fsm->state)
    {
    case STATE_R:
        fsm->len = read(fsm->sfd, fsm->buf, BUFSIZE);
        if(fsm->len == 0)
            fsm->state = STATE_T;
        else if(fsm->len < 0)
        {
            if(errno == EAGAIN)
                fsm->state = STATE_R;
            else
            {
                fsm->errstr = "read()";
                fsm->state = STATE_Ex;
            }
        }
        else 
        {   
            fsm->pos = 0;
            fsm->state = STATE_W;
        }
        break;

    case STATE_W:
        ret = write(fsm->dfd, fsm->buf+fsm->pos, fsm->len);
        if(ret < 0)
        {
            if(errno == EAGAIN)
                fsm->state = STATE_W;
            else 
            {
                fsm->errstr = "write()";
                fsm->state = STATE_Ex;
            }    
        }
        else
        {
            fsm->pos += ret;
            fsm->len -= ret;
            if(fsm->len == 0)
                fsm->state=STATE_R;
            else 
                fsm->state=STATE_W;
        }
        break;
    case STATE_Ex:
        perror(fsm->errstr);
        fsm->state = STATE_T;
        break;
    case STATE_T:
        abort();
        break;
    default:
        break;
    }
}

static void relay(int fd1, int fd2)
{
    int fd1_save, fd2_save;
    struct fsm_st fsm12, fsm21;

    fd1_save = fcntl(fd1, F_GETFL);
    fcntl(fd1, F_SETFL, fd1_save|O_NONBLOCK); // 设置为非阻塞

    fd2_save = fcntl(fd2, F_GETFL);
    fcntl(fd2, F_SETFL, fd2_save|O_NONBLOCK); // 设置为非阻塞

    // 设置状态机
    fsm12.state = STATE_R;
    fsm12.sfd = fd1;
    fsm12.dfd = fd2;

    fsm21.state = STATE_R;
    fsm21.sfd = fd2;
    fsm21.dfd = fd1;
    // 如果不是终止状态，就让状态机运行
    while(fsm12.state != STATE_T || fsm21.state!=STATE_T)
    {
        fsm_driver(&fsm12);
        fsm_driver(&fsm21);
    }

    // 恢复原来的状态
    fcntl(fd1, F_SETFL, fd1_save);
    fcntl(fd2, F_SETFL, fd2_save);
}

int rel_addjob(int fd1, int fd2);
{
    struct rel_job_st *me;
    me = malloc(sizeof(*me));
    if (me == NULL)
        return -ENOMEM;
    me->fd1 = fd1;
    me->fd2 = fd2;
    me->job_state = STATE_RUNNING;

    me->fd1_save = fcntl(me->fd1, F_GETFL);
    fcntl(me->fd1, F_SETFL, me->fd1_save|O_NONBLOCK);
    me->fd2_save = fcntl(me->fd2, F_GETFL);
    fcntl(me->fd2, F_SETFL, me->fd2_save|O_NONBLOCK);    
}

int rel_canceljob(int id);


int rel_waitjob(int id, struct rel_stat_st *st);


int rel_statjob(int id, struct rel_stat_st *st);
