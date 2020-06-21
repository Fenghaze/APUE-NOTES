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



# 3 IO多路转接

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

- nfds：监视的最大文件描述符+1
- readfds：读文件描述符的集合
- writefds：写文件描述符的集合
- exceptfds：异常文件描述符的集合
- timeout：超时设置

如果成功，返回监视的文件描述符个数；如果失败，返回-1和errno



> 例子：select.c



缺点：监视的文件描述符类型太少

优点：可移植性好



## 3.2 poll

`int poll(struct pollfd *fds, nfds_t nfds, int timeout);`

- fds：存放了文件描述符的结构体数组的起始位置
- nfds：数组中文件描述符个数
- timeout：超时设置



> 例子：poll.c



## 3.3 epoll

```c
// 1、创建一个epoll实例
int epoll_create(int size); 

// 2、操作一个epoll实例
int  epoll_ctl(int  epfd,  int  op,  int  fd,  struct epoll_event *event);

//
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```





# 其他读写函数



# 存储映射IO



# 文件锁



