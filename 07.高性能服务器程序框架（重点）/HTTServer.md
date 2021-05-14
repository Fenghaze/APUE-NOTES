# 半同步/半反应堆线程池

<img src="D:\mynotes\APUE-NOTES\07.高性能服务器程序框架（重点）\assets\半同步版反应堆.png" style="zoom:67%;" />

# HTTPServer设计





## 线程池类

使用模板类，方便各类型服务调用线程池

- 构造函数：
  - （1）创建 m_thread_number个线程的线程池m_threads
  - （2）将每个线程进行分离，后续无需对这些工作线程进行回收
- 成员变量：请求队列`list<T *> m_workqueue`
  - 往请求队列中添加任务时，需要==使用互斥锁保护==请求队列
- 成员函数：`run()`
  - ==等待信号量==`m_queuestat`来判断请求队列中是否有任务
  - 获得请求队列中的服务任务（类型T的服务类）时，需要进行==加锁==
  - 执行服务类的处理函数

> locker.h

```c++
#ifndef __LOCKER_H
#define __LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>
/*封装信号量的类：信号量的作用是可以让多个线程读取资源，达到资源共享*/
class sem
{
public:
    /*创建并初始化信号量*/
    sem()
    {
        if (sem_init(&m_sem, 0, 0) != 0)
        {
            /*构造函数没有返回值，可以通过抛出异常来报告错误*/
            throw std::exception();
        }
    }
    /*销毁信号量*/
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    /*等待信号量*/
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    /*增加信号量*/
    bool post()
    {
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};
/*封装互斥锁的类：互斥锁的作用是保护某个资源，某时刻只允许一个线程访问资源*/
class locker
{
public:
    /*创建并初始化互斥锁*/
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    /*销毁互斥锁*/
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    /*获取互斥锁*/
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    /*释放互斥锁*/
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

private:
    pthread_mutex_t m_mutex;
};
/*封装条件变量的类：条件变量通常与互斥锁一起使用，当资源可用时，用于通知其他线程来竞争资源*/
class cond
{
public:
    /*创建并初始化条件变量*/
    cond()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            /*构造函数中一旦出现问题，就应该立即释放已经成功分配了的资源*/
            pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    /*销毁条件变量*/
    ~cond()
    {
        pthread_mutex_destroy(&m_mutex);
        pthread_cond_destroy(&m_cond);
    }
    /*等待条件变量*/
    bool wait()
    {
        int ret = 0;
        pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, &m_mutex);
        pthread_mutex_unlock(&m_mutex);
        return ret = 0;
    }
    /*唤醒等待条件变量的线程*/
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }

private:
    pthread_mutex_t m_mutex;    /*条件变量的互斥锁*/
    pthread_cond_t m_cond;      /*条件变量*/
};

#endif
```



> threadpool.h

```c++
#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

//线程池类，定义为模板类，模板参数T是任务类
template <typename T>
class threadpool
{

public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T *request);

private:
    static void *worker(void *arg);
    void run();

private:
    int m_thread_number;        //线程池中的线程数量
    int m_max_requests;         //请求队列中允许的最大请求数
    pthread_t *m_threads;       //线程池
    std::list<T *> m_workqueue; //请求队列
    locker m_queuelocker;       //请求队列的互斥锁，保护请求队列
    sem m_queuestat;            //请求队列的信号量，判断请求队列是否有任务
    bool m_stop;                //是否结束线程
};

//定义构造函数：创建线程池，并分离线程
template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests)
    : m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(nullptr)
{
    if ((thread_number <= 0) || (max_requests < 0))
    {
        throw std::exception();
    }
    //初始化线程池
    m_threads = new pthread_t[m_thread_number];
    if (!m_threads)
    {
        throw std::exception();
    }
    //创建线程,并将它们都线程分离：之后不需要回收线程
    for (size_t i = 0; i < m_thread_number; i++)
    {
        printf("create the %dth thread\n", i);
        //创建线程
        if (pthread_create(m_threads + i, NULL, worker, this) != 0)
        {
            delete[] m_threads;
            throw std::exception();
        }
        //线程分离
        if (pthread_detach(m_threads[i]))
        {
            delete[] m_threads;
            throw std::exception();
        }
    }
}

//析构：释放资源，并停止工作线程
template <typename T>
threadpool<T>::~threadpool()
{
    delete[] m_threads;
    m_stop = true;
}

//往请求队列中添加任务：使用互斥锁保护请求队列
template <typename T>
bool threadpool<T>::append(T *request)
{
    //加锁
    m_queuelocker.lock();
    //判断请求队列的任务数量
    if (m_workqueue.size() > m_max_requests)
    {
        //解锁
        m_queuelocker.unlock();
        return false;
    }
    //添加任务
    m_workqueue.push_back(request);
    //解锁
    m_queuelocker.unlock();
    return true;
}

//工作线程
template <typename T>
void *threadpool<T>::worker(void *arg)
{
    //强转为 threadpool* 类型
    threadpool *pool = (threadpool *)arg;
    //执行任务
    pool->run();
    return pool;
}

//工作线程任务：
template <typename T>
void threadpool<T>::run()
{
    while (!m_stop)
    {
        //等待资源
        m_queuestat.wait();
        //加锁
        m_queuelocker.lock();
        if (m_workqueue.empty())
        {
            //解锁
            m_queuelocker.unlock();
            continue;
        }
        //获得任务
        T *request = m_workqueue.front();
        //出队
        m_workqueue.pop_front();
        //解锁
        m_queuelocker.unlock();
        if (!request)
        {
            continue;
        }
        //处理任务
        request->process();
    }
}

#endif
```

## 服务类-HTTP解析服务

> http.h

```c++
/**
  * @file    :http.h
  * @author  :zhl
  * @date    :2021-04-15
  * @desc    :HTTP类，封装了HTTP服务的相关变量和逻辑处理方法
*/

#ifndef __HTTP_H
#define __HTTP_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/uio.h>

#include "locker.h"

class HTTPConn
{

public:
    /*文件名的最大长度*/
    static const int FILENAME_LEN = 200;
    /*读缓冲区的大小*/
    static const int READ_BUFFER_SIZE = 2048;
    /*写缓冲区的大小*/
    static const int WRITE_BUFFER_SIZE = 1024;
    /*HTTP请求方法，但我们仅支持GET*/
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };
    /*解析客户请求时，主状态机所处的状态*/
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    /*服务器处理HTTP请求的可能结果*/
    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    /*行的读取状态*/
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

public:
    HTTPConn() {}
    ~HTTPConn() {}

    //初始化客户连接
    void init(int sockfd, const sockaddr_in &addr);
    //关闭连接
    void close_conn(bool real_close = true);
    //处理客户请求
    void process();
    //非阻塞读
    bool Read();
    //非阻塞写
    bool Write();

private:
    //初始化连接，私有方法
    void init();

    //解析HTTP请求
    HTTP_CODE process_read();
    //下面这一组函数被process_request调用以分析HTTP请求
    LINE_STATUS parse_line();
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return m_read_buf + m_start_line; }

    //生成HTTP响应
    bool process_write(HTTP_CODE res);
    //下面这一组函数被process_response调用以生成HTTP响应
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    /*所有socket上的事件都被注册到同一个epoll内核事件表中，所以将epoll文件描述符设置为静态的*/
    static int m_epollfd;
    /*统计用户数量*/
    static int m_user_count;

private:
    /*该HTTP连接的socket和对方的socket地址*/
    int m_sockfd;
    sockaddr_in m_address;
    /*读缓冲区*/
    char m_read_buf[READ_BUFFER_SIZE];
    /*标识读缓冲中已经读入的客户数据的最后一个字节的下一个位置*/
    int m_read_idx;
    /*当前正在分析的字符在读缓冲区中的位置*/
    int m_checked_idx;
    /*当前正在解析的行的起始位置*/
    int m_start_line;
    /*写缓冲区*/
    char m_write_buf[WRITE_BUFFER_SIZE];
    /*写缓冲区中待发送的字节数*/
    int m_write_idx;
    /*主状态机当前所处的状态*/
    CHECK_STATE m_check_state;
    /*请求方法*/
    METHOD m_method;
    /*客户请求的目标文件的完整路径，其内容等于doc_root+m_url，doc_root是网站根目录*/
    char m_real_file[FILENAME_LEN];
    /*客户请求的目标文件的文件名*/
    char *m_url;
    /*HTTP协议版本号，我们仅支持HTTP/1.1*/
    char *m_version;
    /*主机名*/
    char *m_host;
    /*HTTP请求的消息体的长度*/
    int m_content_length;
    /*HTTP请求是否要求保持连接*/
    bool m_linger;
    /*客户请求的目标文件被mmap到内存中的起始位置*/
    char *m_file_address;
    /*目标文件的状态。通过它我们可以判断文件是否存在、是否为目录、是否可读，并获取文件大小等信息*/
    struct stat m_file_stat;
    /*我们将采用writev来执行写操作，所以定义下面两个成员，其中m_iv_count表示被写内存块的数量*/
    struct iovec m_iv[2];
    int m_iv_count;
};

#endif
```



> http.cc

```c++

```



