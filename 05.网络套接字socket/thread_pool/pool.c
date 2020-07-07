#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>

#define DEFAULT_TIME 10
#define MIN_WAIT_TASK_NUM 10
#define DEFAULT_THREAD_VARY 10

typedef struct
{
    void *(*function)(void *); // 函数指针
    void *arg;                 // 回调函数参数
} threadpool_task_t;           // 线程的任务结构体

// 线程池结构体
struct threadpool_t
{
    pthread_mutex_t lock;           // 锁住本结构体
    pthread_mutex_t thread_counter; // 记录正在工作线程个数的锁
    pthread_cond_t queue_not_full;  // 当任务队列满时，添加任务的线程阻塞，等待此条件变量
    pthread_cond_t queue_not_empty; // 当任务队列不为空时，通知所有等待任务的线程

    pthread_t *threads;            // 线程池的线程数组
    pthread_t adujst_tid;          // 管理线程池的线程
    threadpool_task_t *task_queue; // 任务队列

    int min_thr_num;       // 线程池下限
    int max_thr_num;       // 线程池上限
    int live_thr_num;      // 当前线程池存活个数
    int busy_thr_num;      // 当前线程池工作个数
    int wait_exit_thr_num; // 要销毁的线程个数

    int queue_front;    //任务队列队头
    int queue_rear;     //任务队列队尾
    int queue_size;     //任务队列中任务个数
    int queue_max_size; //任务队列上限

    int shutdown; // 标志位，线程使用状态，0\1
};

// 工作线程的处理函数
void threadpool_thread(void *threadpool)
{
    threadpool_t *pool = (threadpool_t *)threadpool;
    threadpool_task_t task;

    while (1)
    {
        // 要操作线程池时，需要加锁
        pthread_mutex_lock(&(pool->lock));
        // 当任务队列没有任务且线程池没有关闭时，等待唤醒接受任务
        // 当任务队列有任务时，跳过
        while ((pool->queue_size == 0) && (!pool->shutdown))
        {
            printf("thread 0x%x is waiting..\n", pthread_self());
            // 阻塞等待，解锁线程池
            // 当有人通知任务队列有新的任务时，加锁线程池，继续向下执行
            pthread_cond_wait(&(pool->queue_not_empty), &(pool->lock));

            if (pool->wait_exit_thr_num > 0)
            {
                pool->wait_exit_thr_num--;

                if (pool->live_thr_num > pool->min_thr_num)
                {
                    printf("thread 0x%x is exiting..\n", pthread_self());
                    pool->live_thr_num--;
                    pthread_mutex_unlock(&(pool->lock));
                    pthread_exit(NULL);
                }
            }
        }

        if (pool->shutdown)
        {
            pthread_mutex_unlock(&(pool->lock));
            printf("thread 0x%x is exiting.\n", pthread_self());
            pthread_exit(NULL);
        }

        // 从结构体中的任务队列中获取任务，任务出队
        task.function = pool->task_queue[pool->queue_front].function;
        task.arg = pool->task_queue[pool->queue_front].arg;
        // 模拟环形队列
        pool->queue_front = (pool->queue_front + 1) % pool->queue_max_size;
        pool->queue_size--;

        // 通知外部，可以有新的任务加入到任务队列
        pthread_cond_broadcast(&(pool->queue_not_full));
        // 解锁
        pthread_mutex_unlock(&(pool->lock));

        // 执行任务
        printf("thread 0x%x start working...\n", pthread_self());

        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num++;
        pthread_mutex_unlock(&(pool->thread_counter));

        (*(task.function))(task.arg); // 任务的回调函数

        // 任务结束处理，正在运行的线程-1
        printf("thread 0x%x end working\n", pthread_self());
        pthread_mutex_lock(&(pool->thread_counter));
        pool->busy_thr_num--;
        pthread_mutex_unlock(&(pool->thread_counter));
    }
    pthread_exit(NULL);
}

threadpool_t *threadpool_create(int min_thr_num, int max_thr_num, int queue_max_size)
{
    int i;
    threadpool_t *pool = NULL;
    do
    {
        if ((pool = (threadpool_t *)malloc(threadpool_t)))== NULL)
            {
                printf("malloc failed...");
                break;
            }

        pool->min_thr_num = min_thr_num;
        pool->max_thr_num = max_thr_num;
        pool->busy_thr_num = 0;
        pool->live_thr_num = min_thr_num;

        pool->queue_size = 0;
        pool->queue_max_size = queue_max_size;
        pool->queue_front = 0;
        pool->queue_rear = 0;

        pool->shutdown = false; // 不关闭线程池

        // 线程数组的空间要能存放最大上限的线程数
        pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * max_thr_num);
        if (pool->threads == NULL)
        {
            printf("malloc threads failed");
            break;
        }
        // 清零
        memset(pool->threads, 0, sizeof(pthread_t) * max_thr_num);

        // 任务队列开辟空间
        pool->task_queue = (threadpool_task_t *)malloc(sizeof(threadpool_task_t) * queue_max_size);
        if (pool->task_queue == NULL)
        {
            printf("malloc task queue failed");
            break;
        }

        // 初始化互斥锁、条件变量
        if (pthread_mutex_init(&(pool->lock), NULL) != 0 || pthread_mutex_init(&(pool->thread_counter), NULL) != 0 || pthread_cond_init(&(pool->queue_not_empty), NULL) != 0 || pthread_cond_init(&(pool->queue_not), NULL) != 0)
        {
            printf("init lock or cond failed");
            break;
        }

        // 启动最小数量的线程
        for (i = 0; i < min_thr_num; i++)
        {
            pthread_create(&pool->threads[i], NULL, threadpool_thread, (void *)pool);
            printf("start thread 0x%x...\n", pool->threads[i]);
        }
        // 启动管理线程池的线程
        pthread_create(&(pool->adjust_tid), NULL, adjust_thread, (void *)pool);
        return pool;
    } while (0);

    threadpool_free(pool);
}
void *process(void *arg)
{
    printf("thread 0x%x working on task %d\n", pthread_self(), (int)arg);
    sleep(1);
    printf("task %d is end\n", (int)arg);
    return NULL;
}

// 管理线程池的线程
void *adjust_thread(void *threadpool)
{
    int i;
    threadpool_t *pool = (threadpool_t *)threadpool;
    while (!pool->shutdown)
    {
        // 定时对线程池进行管理
        sleep(DEFAULT_TIME);

        pthread_mutex_lock(&(pool->lock));
        int queue_size = pool->queue_size;
        int live_thr_num = pool->live_thr_num;
        pthread_mutex_unlock(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        int busy_thr_num = pool->busy_thr_num;
        pthread_mutex_unlock(&(pool->thread_counter));

        // 扩张线程池的算法
        if (queue_size >= MIN_WAIT_TASK_NUM && live_thr_num < pool->max_thr_num)
        {
            pthread_mutex_lock(&(pool->lock));
            int add = 0;
            // 一次扩张固定线程个数
            for (i = 0; i < pool->max_thr_num && add < DEFAULT_THREAD_VARY && pool->live_thr_num < pool->max_thr_num; i++)
            {
                if (pool->threads[i] == 0 || !is_thread_alive(pool->threads[i]))
                {
                    pthread_create(&(pool->threads[i]), NULL, threadpool_thread, (void *)pool);
                    add++;
                }
            }
            pthread_mutex_unlock(&(pool->lock));
        }

        // 缩小线程池的算法
        if (busy_thr_num * 2 < live_thr_num && live_thr_num > pool->min_thr_num)
        {
            pthread_mutex_lock(&(pool->lock));
            pool->wait_exit_thr_num = DEFAULT_THREAD_VARY;

            pthread_mutex_unlock(&(pool->lock));
            for (i = 0; i < DEFAULT_THREAD_VARY; i++)
            {
                pthread_cond_signal(&(pool->queue_not_empty));
            }
        }
    }
    return NULL;
}

// 向任务队列添加任务
int thread_add(threadpool_t *pool, void *(*function(void *arg)), void *arg)
{
    pthread_mutex_lock(&(pool->lock));

    // 为真，队列满，阻塞
    while ((pool->queue_size == pool->queue_max_size) && (!pool->shutdown))
    {
        // 解锁，等待条件变量被唤醒
        // 当有通知发过来说队列不为空了，加锁，跳出循环继续向下执行
        pthread_cond_wait(&(pool->queue_not_full), &(pool->lock));
    }
    if (pool->shutdown)
        pthread_mutex_unlock(&(pool->lock));

    if (pool->task_queue[pool->queue_rear].arg != NULL)
    {
        free(pool->task_queue[pool->queue_rear], arg);
        pool->task_queue[pool->queue].arg = NULL;
    }

    // 添加任务到任务队列
    pool->task_queue[pool->queue_rear].function = function;
    pool->task_queue[pool->queue].arg = arg;
    pool->queue_rear = (pool->queue_rear + 1) % pool->queue_max_size;
    pool->queue_size++;

    // 添加了任务后，通知线程池中等待任务的线程，有任务了
    pthread_cond_signal(&(pool->queue_not_empty));
    pthread_mutex_unlock(&(pool->lock));

    return 0;
}

// 消耗线程池
int threadpool_free(threadpool_t *pool)
{
    if(pool ==NULL)
        return -1;
    if(pool->task_queue)
        free(pool->task_queue);
    if (pool->threads)
    {
        free(pool->threads);
        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));

        pthread_mutex_lock(&(pool->thread_counter));
        pthread_mutex_destroy(&(pool->thread_counter));

        pthread_cond_destroy(&(pool->queue_not_empty));
        pthread_cond_destroy(&(pool->queue_not_full));
    }
    free(pool);
    pool = NULL;
    return 0;
    
}

// 销毁所有线程
int threadpool_destroy(threadpool_t *pool)
{
    int i;
    if(pool ==NULL)
        return -1;
    pool->shutdown = true;
    // 先销毁管理线程
    pthread_join(pool->just_tid, NULL);
    for ( i = 0; i < pool->live_thr_num; i++)
    {
        pthread_cond_broadcast(&(pool->queue_not_empty));
    }
    for ( i = 0; i < pool->threas[i]; i++)
    {
        pthread_join(pool->threads[i], NULL);
    }
    threadpool_free(pool);
    return 0;
    
}

int threadpool_all_threadnum(threadpool_t *pool)
{
    int all_threadnum = -1;
    pthread_mutex_lock(&(pool->lock));
    all_threadnum = pool->live_thr_num;
    pthread_mutex_unlock(&(pool->lock));
    return all_threadnum;
}

int threadpool_busy_threadnum(threadpool_t *pool)
{
    int busy_threadnum = -1;
    pthread_mutex_lock(&(pool->thread_counter));
    all_threadnum = pool->busy_thr_num;
    pthread_mutex_unlock(&(pool->thread_counter));
    return busy_threadnum;
    
}

int is_thread_alive(pthread_t tid)
{
    int kill_rc = pthread_kill(tid, 0);
    if (kill_rc == ESRCH)
        return false;
    return true;
}

int main()
{
    // 创建线程池
    threadpool_t *thp = threadpool_create(3, 100, 100);
    printf("pool inited\n");

    int num[20], i;
    for (i = 0; i < 20; i++)
    {
        num[i] = i;
        printf("add task %d\n", i);
        // 任务队列添加任务，线程池接受并处理任务
        threadpool_add(thp, process, (void *)&num[i]);
    }
    sleep(10);
    threadpool_destroy(thp);

    return 0;
}
