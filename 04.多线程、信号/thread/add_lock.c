#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<string.h>

#define THRNUM      20
#define FNAME       "/tmp/out"
#define LINESIZE    1024

static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

static void *thr_add(void *p)
{
    // 每个线程读写同一个文件
    FILE * fp  = fopen(FNAME, "r+");
    char linebuf[LINESIZE];
    if(!fp)
    {
        perror("fopen()");
        exit(1);
    }
    pthread_mutex_lock(&mut);
    fgets(linebuf, LINESIZE, fp);
    fseek(fp, 0, SEEK_SET); //定位
    fprintf(fp,"%d\n", atoi(linebuf)+1);
    fclose(fp);
    pthread_mutex_unlock(&mut);
    pthread_exit(NULL);
}


int main()
{
    pthread_t tid[THRNUM];
    int i, err;
    for ( i = 0; i < THRNUM; i++)
    {
        // 创建20个线程
        err = pthread_create(tid+i, NULL, thr_add, NULL);
        if(err)
        {
            fprintf(stderr, "%s\n", strerror(err));
            exit(1);
        }   /* code */
    }
    
    for ( i = 0; i < THRNUM; i++)
    {
        pthread_join(tid[i], NULL);               /* code */
    }
    
    pthread_mutex_destroy(&mut);
    exit(0);

}