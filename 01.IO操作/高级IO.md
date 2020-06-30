# 1 非阻塞IO

两个设备之间的数据交换称为数据中继



假设有这样一个数据交换的任务：

读设备A->将读取到的内容写入到设备B->读设备B->将读取到的内容写入到设备A



对于单线程（单进程）来说

阻塞IO：如果设备A没有内容，就会一直阻塞在读设备A的位置，由于设备A被阻塞且只有一个线程在工作，那么后续任务无法执行下去

非阻塞IO：**如果设备A没有内容，那么就会放弃这一步，去执行下面的操作，不会卡死在某一个环节**



对于多线程（多进程）来说：将任务进行划分，一个线程执行读设备A，写到设备B的任务；一个线程执行读设备B，写到设备A的任务

阻塞IO：即便有一个任务阻塞了，也不会影响到整体的任务循环



# 2 有限状态机编程思路

自然流程：解决问题的流程图

简单流程：一个程序的自然流程是结构化的（顺序流程图可以描述这个任务）

复杂流程：一个程序的自然流程不是结构化的（不能用简单的顺序流程图描述整个任务）



有限状态机用于解决复杂流程，一般分为以下步骤：

- 程序开始
- 如果出现真错，EX态，程序终止
- 如果出现假错，E态，程序重新开始

- 任务完成，程序终止



> ./src/nonblock/relay.c 有限状态机流程图

![](./src/nonblock/有限状态机realy.png)



# 3 ==IO多路转接==

实现文件描述符的监视，当文件描述符状态发生改变时，才去推动状态机，而不是盲目等待

```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);


int poll(struct pollfd *fds, nfds_t nfds, int timeout);


epoll; // 是poll在Linux封装的方言
```



==大致步骤：==

- 1、布置监视任务
- 2、选择监视函数进行监视
- 3、根据监视的结果来执行相应的操作



## 3.1 select

`int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);`

fd_set是位图

- nfds：监视的最大文件描述符+1
- readfds：读文件描述符的集合（监听集合中的文件描述符的读事件）
- writefds：写文件描述符的集合
- exceptfds：异常文件描述符的集合
- timeout：超时设置

如果成功，返回监听集合中满足条件的文件描述符个数；如果失败，返回-1和errno



问题1：如何将文件描述符添加到各个集合中？

先调用`FD_ZERO`，再`FD_SET`

问题2：对于成功返回的文件描述符个数，如何区分哪个fd来自与哪个集合？

for循环，调用`FD_ISSET`来判断每个集合中是否存在fd

```c
void FD_ZERO(fd_set *set);			//将集合清空

void FD_CLR(int fd, fd_set *set);	//将一个fd从set中清除

void FD_SET(int fd, fd_set *set);	//将一个fd添加到set中

int  FD_ISSET(int fd, fd_set *set);	//判断fd是否在集合中
```



> 例子：select.c



缺点：监视的文件描述符类型太少；同时监听的文件描述符有上限（1024）；

优点：可移植性好



## 3.2 poll

`int poll(struct pollfd *fds, nfds_t nfds, int timeout);`

- fds：存放了文件描述符及其事件的结构体数组的起始地址
- nfds：数组中文件描述符个数
- timeout：超时设置

```c
struct pollfd {
    int   fd;         /* file descriptor */
    short events;     /* 要监听的事件：POLLIN/POLLOUT/POLLERR */
    short revents;    /* 当监听到某事件后，会自动赋值 */
};
```



> 例子：poll.c



优点：没有监听上限；监听、返回集合分离；



## 3.3 ==epoll==

epoll是Liunx的方言，是select/poll的增强版本

epoll的存储结构是一棵**红黑树**

```c
// 1、创建epoll句柄
int epoll_create(int size); 
- size：监听的文件描述符个数
成功，返回一个epfd（epfd是一棵红黑树的根节点，有size个文件描述符作为结点）

// 2、设置epoll
int  epoll_ctl(int  epfd,  int  op,  int  fd,  struct epoll_event *event);
- op：对红黑树中的结点进行的操作（增加/删除文件描述符结点）
- event：监听事件结构体的地址

// 3、监听等待
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
- events：监听事件结构体数组的首地址
- maxevents：数组容量
- timeout：超时等待
成功，返回监听到的文件描述符个数，结构体数组中自动存放了监听到的fd及其事件（要使用for循环执行后来的操作）

// 结构体
typedef union epoll_data {
    void        *ptr;	// 应用于epoll反应堆模型
    int          fd;
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* EPOLLIN/OUT/ERR */
    epoll_data_t data;        /* User data variable */
};
```



### epoll触发模式

- 边沿触发：epoll ET
- 水平触发：epoll LT



### epoll 反应堆模型





# 4 readv、writev

从多个碎片的小地址读或写

```c
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```



# 5 存储映射IO

把磁盘的文件的某块内容映射到当前进程虚拟空间中，==可用于进程间通信==



## 5.1 mmap、munmap

`void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);`

- addr：指定映射到的进程空间的起始位置，如果为NULL，让系统自己分配
- length：映射区长度
- prot：映射区的操作权限（读/写）
- flags：权限（共享、私有）
- fd：文件描述符，打开要映射的文件
- offset：指定要映射文件的偏移量

如果成功，返回创建的映射区首地址；失败，返回MAP_FAILED宏



`int munmap(void *addr, size_t length);`

解除映射



> 【示例1】映射文件到当前进程：mmap.c

> 【示例2】使用匿名映射到父子进程，子进程写父进程读：fork_mmap.c
>
> 【示例3】无血缘关系进程间通信



==注意事项：==

- 映射区大小不能为0（不能在open的时候使用O_CREATE创建一个**空文件**映射到进程空间）
- 映射区的权限 **<=** 文件打开权限
- 创建映射区时，文件至少需要有**读的权限**
- 文件偏移量参数，必须是4K的整数倍（CPU的MMU单元负责内存映射，映射单位是4K）



## 5.2 匿名映射区

在之前的映射时，每次创建映射区一定要依赖一个打开的文件才能实现

通常为了建立一个映射区，要open一个文件，创建好映射区后，再close文件，比较麻烦

Linux系统提供了创建匿名映射区的办法，无需依赖一个文件即可创建映射区，需要指定flags参数

flags=`MAP_SHARED|MAP_ANONYMOUS`

```c
int *p = mmap(NULL, 4, PROT_READ, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
// 映射区长度任意；文件描述符为-1，不使用文件
```



由于这个匿名映射的宏是Linux提供的，因此不适用于其他操作系统

这时，可以使用`/dev/zero`文件来创建映射区，这个文件的大小是无限大的，给一指定任意长度，和`/dev/null`文件相对应

```c
fd = open("/dev/zero", O_RDWR);
p = mmap(NULL, size, PROT_READ, MMAP_SHARED, fd, 0);
```



# 6 文件锁

对文件进行加锁，防止产生竞争冲突

```c
int fcntl(int fd, int cmd, ... /* arg */ );

int lockf(int fd, int cmd, off_t len);

int flock(int fd, int operation);
```



> 【示例】20个子进程写同一个文件：add.c

