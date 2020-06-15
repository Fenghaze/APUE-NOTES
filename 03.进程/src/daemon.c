#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<syslog.h>
#include<errno.h>
#define FNAME "/tmp/out"

static int deamonize()
{
    int pid = fork();
    if(pid < 0)
        return -1;
    if(pid > 0) // 父进程退出
        exit(0);

    // 子进程
    int fd = open("/dev/null", O_RDWR);
    if(fd < 0)
        return -1;
    dup2(fd, 0);
    dup2(fd, 1);
    dup2(fd, 2);// 将stdin,stdout,stderr重定向到fd
    if(fd>2)
        close(fd);
    setsid(); // 将子进程设置为守护进程
    chdir("/"); // 切换守护进程的工作路径为根目录
    return 0;
}
 

int main()
{
    FILE *fp;
    openlog("mydaemon", LOG_PID, LOG_DAEMON);
    if(deamonize())
    {
        syslog(LOG_ERR, "daemonize() failed!");
        exit(1);
    }
    else
    {
        syslog(LOG_INFO, "daemonize() successded!");
    }
    
    fp = fopen(FNAME, "w");
    if (fp == NULL)
    {
        syslog(LOG_ERR, "fopen(): %d", strerror(errno));
        exit(1);   /* code */
    }
    syslog(LOG_INFO, "%s was opened.", FNAME);
    for(int i=0; ;i++)
    {
        fprintf(fp, "%d\n", i);
        fflush(fp);
        syslog(LOG_DEBUG, "%d is printed.", i);
        sleep(1);   /* code */
    }
    fclose(fp);
    closelog();
    exit(0);
}