#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>

#define TTY1    "/dev/tty11"
#define TTY2    "/dev/tty12"
#define TTY3    "/dev/tty9"
#define TTY4    "/dev/tty10"

#define BUFSIZE 1024

struct fsm_st
{
    int state;
    int sfd;
    int dfd;
    char buf[BUFSIZE];
    int len;
    int pos;
    char *errstr;
};

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


int main()
{
    int fd1, fd2;
    int fd3, fd4;
    int job1, job2;
    fd1 = open(TTY1, O_RDWR);
    if (fd1<0)
    {
        perror("open()");
        exit(1);   
    }
    write(fd1, "TTY1\n", 5);
    fd2 = open(TTY1, O_RDWR|O_NONBLOCK); //非阻塞方式打开
    if (fd2<0)
    {
        perror("open()");
        exit(1);   
    }
    write(fd2, "TTY2\n", 5);
    job1 = rel_addjob(fd1, fd2);
    if (job1 < 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);   /* code */
    }

    fd3 = open(TTY3, O_RDWR);
    if (fd3<0)
    {
        perror("open()");
        exit(1);   
    }
    write(fd3, "TTY3\n", 5);
    fd4 = open(TTY4, O_RDWR|O_NONBLOCK); //非阻塞方式打开
    if (fd4<0)
    {
        perror("open()");
        exit(1);   
    }
    write(fd4, "TTY4\n", 5);
    job2 = rel_addjob(fd3, fd4);
    if (job2 < 0)
    {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);   /* code */
    }
    
    while (1)
        pause();   /* code */
    
    close(fd2);
    close(fd1);
    close(fd3);
    close(fd4);
    


    exit(0);
}