进程是相互独立的，所以进程间访问同一资源大多不需要锁，需要的锁也是文件锁之类的“大锁”，并不需要线程中的条件变量、互斥锁这些机制防止出现资源竞争；因此大多数情况下，通常采用**不同的通信方式来传递资源**，因此进程间关注的是通信方式，而不是同步方式

进程同步的方式：

- 信号量
- 文件锁
- 互斥锁

进程间通信的方式：

- 管道
- 内存映射IO
- 共享内存
- 消息队列



# 进程同步

## 1 信号量

多个进程同时访问某个资源时，需要考虑进程同步问题

信号量（Semaphore Value）是一种特殊的变量，只能取自然数值。SV=n，表示允许n个进程来读取这个资源

信号量支持两种操作：等待（wait）和信号（signal）；在Linux中通常称为P、V操作

P(SV)：当某个进程来读取资源时，SV-1；当SV=0时，表示当前读取资源的进程上限已经达到，需要等待资源，处于挂起状态，直到V唤醒它

V(SV)：当某个进程使用完资源，退出临界区时，信号量SV+1；并通知其他挂起等待的进程来争夺资源



> 创建一个新的信号集或获取一个已经存在的信号集

```c++
int semget(key_t key, int num_sems, int sem_flags);
- key：键值，用于标识信号集
- num_sems：创建/获取的信号量数目；如果是获取已存在的信号量，设置为0
- sem_flags:标志位，与open()的mode参数相同，权限
```

成功时返回一个正整数值，它是信号量集的标识符；semget失败时返回-1，并设置errno



如果semget用于**创建信号量集**，则与之关联的内核数据结构体semid_ds将被创建并初始化。semid_ds结构体的定义如下：

```c++
#include＜sys/sem.h＞
/*该结构体用于描述IPC对象（信号量、共享内存和消息队列）的权限*/
struct ipc_perm
{
    key_t key;/*键值*/
    uid_t uid;/*所有者的有效用户ID*/
    gid_t gid;/*所有者的有效组ID*/
    uid_t cuid;/*创建者的有效用户ID*/		//初始化
    gid_t cgid;/*创建者的有效组ID*/		//初始化
    mode_t mode;/*访问权限*/			  //初始化
    /*省略其他填充字段*/
}
struct semid_ds
{
    struct ipc_perm sem_perm;/*信号量的操作权限*/
    unsigned long int sem_nsems;/*该信号量集中的信号量数目*/	//初始化为num_sems
    time_t sem_otime;/*最后一次调用semop的时间*/					//初始化为0
    time_t sem_ctime;/*最后一次调用semctl的时间*/				//初始化为当前系统时间
    /*省略其他填充字段*/
};
```



==补充：IPC_PRIVATE==

可以给key参数传递一个特殊的键值IPC_PRIVATE（其值为0），这样无论该信号量是否已经存在，semget都将创建一个新的信号量。使用该键值创建的信号量并非像它的名字声称的那样是进程私有的。其他进程，尤其是子进程，也有方法来访问这个信号量。



>  改变信号量的值，即执行P、V操作

`semop`对信号量的操作实际上是对内核中信号量相关的变量进行操作，内核变量如下：

```c++
unsigned short semval;/*信号量的值*/
unsigned short semzcnt;/*等待信号量值变为0的进程数量*/
unsigned short semncnt;/*等待信号量值增加的进程数量*/
pid_t sempid;/*最后一次执行semop操作的进程ID*/
```

------------------------------------------------------------------------------------------------------------------------

```c++
int semop(int sem_id, struct sembuf *sem_ops, size_t num_sem_ops);
- sem_id：semget调用返回的信号量集标识符，指定即将改变的目标信号量集
- sem_ops：指向一个sembuf结构体类型的数组    
- num_sem_ops:指定要执行的操作个数，即sem_ops数组中元素的个数
```

semop对数组sem_ops中的每个成员按照数组顺序依次执行操作，并且该过程是原子操作，以避免别的进程
在同一时刻按照不同的顺序对该信号集中的信号量执行semop操作导致的竞态条件。

成功时返回0，失败则返回-1并设置errno。失败的时候，sem_ops数组中指定的所有操作都不被执行



sem_ops结构体定义如下：

```c++
struct sembuf
{
    unsigned short int sem_num;	/*信号量集中信号量的编号，0表示信号量集中的第一个信号量*/
    short int sem_op;/*指定操作类型，其可选值为正整数、0和负整数。每种类型的操作的行为又受到sem_flg成员的影响*/
    short int sem_flg;  // IPC_NOWAIT(无论信号量操作是否成功，semop调用都将立即返回，这类似于非阻塞I/O操作); SEM_UNDO(当进程退出时取消正在进行的semop操作)
}
```

sem_op和sem_flg将按照如下方式来影响semop的行为：

- sem_op>0：V操作，信号量增加，内核变量semval增加sem_op，该操作要求调用进程对被操作信号集拥有==写权限==
- sem_op=0：表示这是一个“等待0”（wait-for-zero）操作。该操作要求调用进程对被操作信号量集拥有==读权限==。如果此时内核变量semval的值是0，则调用立即成功返回。如果信号量的值不是0，则semop失败返回或者阻塞进程以等待信号量变为0
- sem_op<0：P操作，信号量减少，内核变量semval减少sem_op，该操作要求调用进程对被操作信号集拥有==写权限==。如果semval$≥$sem_op的绝对值，则semop操作成功，调用进程立即获得信号量，并且系统将该信号量的semval值减去sem_op的绝对值



> 对信号量直接控制

```c++
int semctl(int sem_id, int sem_num, int command, ...);
- sem_id:由semget调用返回的信号量集标识符，用以指定被操作的信号量集
- sem_num:指定被操作的信号量在信号量集中的编号
- command:指定要执行的命令
```

成功时的返回值取决于command参数。semctl失败时返回-1，并设置errno。



## 2 文件锁

对文件进行加锁，防止产生竞争冲突

```c
int fcntl(int fd, int cmd, ... /* arg */ );

int lockf(int fd, int cmd, off_t len);

int flock(int fd, int operation);
```



## 3 互斥锁

进程间也可以使用互斥量来达到同步的目的，需要设置相应的属性



# ==进程间通信==

## 1 管道 pipe

管道是一个最基本的IPC（进程间通信）机制，有如下特质：

- 其本质是一个伪文件（内核缓冲区）,不占用磁盘空间
- 由两个文件描述符引用，一个表示读端fd[0],一个表示写端fd[1]
- 数据从管道的写端流入，从读端流出（==半双工==）

管道的原理：管道实为内核使用**环形队列**机制，借助内核缓冲区（4K）实现

管道的局限性：

- 数据自己读，不能自己写
- 数据一旦被读走，便不在管道中存在，不可反复读取
- 采用的是半双工通信方式，即，数据只能在一个方向上流动

.![](./src/2020-06-24 08-55-16 的屏幕截图.png)



### 1.1 普通管道

不存储在磁盘中，**只能用于有血缘关系的进程间通信**

==注意：==

- 管道是在fork之前创建的
- fork之后，父子进程共享同一个管道，且具有管道读、写端的权限
- 根据管道的机制，通信时，每个进程应该==只能有一端的操作权限，因此需要关闭另一端==

```c
int pipe(int pipefd[2]);
- pipefd[0]：读端的文件描述符
- pipefd[1]：写端的文件描述符
```



> 【示例】父子进程使用匿名管道通信：pipe_fork.c



### 1.2 命名管道

磁盘中文件类型为p的文件，就是命名管道，可以让**非血缘关系的进程进行通信**

```c
int mkfifo(const char *pathname, mode_t mode);
```



### 1.3 双向管道

socketpair()函数用于创建一对匿名的、相互连接的套接字。 

```c++
int socketpair(int domain, int type, int protocol, int sv[2]);
- domain：协议族，一般为AF_UNIX
- type：SOCK类型
- protocol：协议号
- sv[2]：创建的2个连接套接字，sv[0]读/写，sv[1]写/读
成功，则返回0，创建好的套接字分别是sv[0]和sv[1]；否则返回-1，错误码保存于errno中。
```

- 读写操作位于同一个进程时，sv[0]用于读，sv[1]用于写
- 读写操作位于不同进程时，读进程需要关闭写端，写进程需要关闭读端



**socketpair与pipe的区别：**

管道pipe是半双工的，pipe两次才能实现全双工，使得代码复杂。socketpair直接就可以实现全双工；

socketpair对两个文件描述符中的任何一个都可读和可写，而pipe是一个读，一个写



## 2 内存映射 mmap

详情见【01.IO操作-高级IO.md】

- 1、创建映射空间

```c++
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
- addr：指定映射到的进程空间的起始位置，如果为NULL，让系统自己分配
- length：映射区长度
- prot：映射区的操作权限（读/写）
- flags：权限（共享、私有）
- fd：文件描述符，打开要映射的文件
- offset：指定要映射文件的偏移量
如果成功，返回创建的映射区首地址；失败，返回MAP_FAILED宏
```

- 2、接触映射

```c++
int munmap(void *addr, size_t length);
```

==注意事项：==

- 映射区大小不能为0（不能在open的时候使用O_CREATE创建一个**空文件**映射到进程空间）
- 映射区的权限 **<=** 文件打开权限
- 创建映射区时，文件至少需要有**读的权限**
- 文件偏移量参数，必须是4K的整数倍（CPU的MMU单元负责内存映射，映射单位是4K）

### 2.1 匿名映射

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



## 3 共享内存

**共享内存是最高效的IPC机制**，因为它不涉及进程之间的任何数据传输，让进程共享同一块内存，使得资源可以重复使用

这种高效率带来的问题是：必须使用其他辅助手段来同步进程对共享内存的访问，否则会产生竞态条件

- 1、创建一段新的共享内存，或者获取一段已经存在的共享内存：

```c++
#include <sys/ipc.h>
#include <sys/shm.h>

int shmget(key_t key, size_t size, int shmflg);
- key：用来标识全局唯一的共享内存
- size：内存的大小，单位是字节；如果=0，则获取已经存在的共享内存
- shmflg：标志位，与semget()的sem_flags参数相同，用于指定权限
成功，返回正整数值，它是共享内存的标识符shm_id；失败返回-1，并设置errno
```

如果shmget用于创建共享内存，则这段共享内存的所有字节都被初始化为0，与之关联的内核数据结构shmid_ds将被创建并初始化。shmid_ds结构体的定义如下：

```c++
struct shmid_ds
{
    struct ipc_perm shm_perm;/*共享内存的操作权限*/
    size_t shm_segsz;/*共享内存大小，单位是字节*/
    __time_t shm_atime;/*对这段内存最后一次调用shmat的时间*/
    __time_t shm_dtime;/*对这段内存最后一次调用shmdt的时间*/
    __time_t shm_ctime;/*对这段内存最后一次调用shmctl的时间*/
    __pid_t shm_cpid;/*创建者的PID*/
    __pid_t shm_lpid;/*最后一次执行shmat或shmdt操作的进程的PID*/
    shmatt_t shm_nattach;/*目前关联到此共享内存的进程数量*/
    /*省略一些填充字段*/
};
```



- 2、共享内存被创建/获取后，需要先将它关联到进程的地址空间

```c++
void *shmat(int shm_id, const void* shm_addr, int shmflg);
- shm_id：shmget的返回值，共享内存的标识符
- shm_addr：进程的地址空间
- shmflg：
    - NULL：则被关联的地址由操作系统自动分配
    - 非空：
    	- SHM_RND标志未被设置：则共享内存被关联到addr指定的地址处
    	- 设置了SHM_RND标志：则被关联的地址是[shm_addr-(shm_addr%SHMLBA)]
```

shmat成功时返回共享内存被关联到的地址，失败则返回(void*)-1并设置errno。shmat成功时，将修改内
核数据结构shmid_ds的部分字段，如下：

- 将shm_nattach加1
- 将shm_lpid设置为调用进程的PID
- 将shm_atime设置为当前的时间



- 3、使用完共享内存后，需要将它从进程空间中分离

```c++
int shmdt(const void* shm_addr);
```

成功时返回0，失败则返回-1并设置errno。shmdt在成功调用时将修改内核数据结构shmid_ds的部分字段，如下：

- 将shm_nattach减1
- 将shm_lpid设置为调用进程的PID
- 将shm_dtime设置为当前的时间



- 可以使用系统调用，控制共享内存的某些属性

```c++
int shmctl(int shm_id, int command, struct shmid_ds *buf);
- shm_id：由shmget调用返回的共享内存标识符
- command：指定要执行的命令
```

shmctl成功时的返回值取决于command参数。shmctl失败时返回-1，并设置errno。



### 3.1 创建共享内存的POSIX方法

使用POSIX方法时，编译时需要制定链接选项`-lrt`:

```shell
g++ -o server server.cc -lrt
```



- 创建或打开一个POSXI共享内存对象：

```c++
int shm_open(const char *name, int oflag, mode_t mode); // 类似open()的mode
- name:创建/打开的共享内存对象
- oflag：标志位，指定创建/打开共享内存的方式
    - O_RDONLY:只读方式
    - O_RDWR：可读可写方式
    - O_CREATE：如果不存在共享内存，则创建
    - O_EXCL：与O_CREATE一起使用，如果name指定的共享内存已存在，则函数返回错误，否则创建新的内存空间
    - O_TRUNC：如果共享内存已存在，则覆盖为新的共享内存
```

成功，返回文件描述符，该文件描述符==可用于mmap()调用==；失败返回-1，并设置errno

```c++
#define USER_LIMIT      5
#define BUFFER_SIZE     1024

static const char *shm_name = "/my_shm";    //共享内存名
int shmfd;  //共享内存fd
char *share_mem = nullptr;  //共享内存首地址

//初始化共享内存，作为所有客户socket连接的读缓冲
shmfd = shm_open(shm_name, O_CREAT|O_RDWR, 0666);
//开辟共享内存空间
int ret = ftruncate(shmfd, USER_LIMIT*BUFFER_SIZE);
//共享内存首地址
share_mem = (char *)mmap(NULL, USER_LIMIT*BUFFER_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, SHMFD, 0);
clsoe(shmfd);
```



- 使用完共享内存对象后，需要进行分离（删除）

```c++
  int shm_unlink(const char *name);
```



### 3.2 内存映射 mmap

mmap函数。利用它的==MAP_ANONYMOUS==标志我们可以实现父、子进程之间的匿名内存共享。通过打开同一个文件，mmap也可以实现无关进程之间的内存共享

详见【01.IO操作/系统调用IO/5.1mmap、munmap】



### 3.3 应用：聊天室程序

【01.高级IO/5 应用/5.2 poll实现聊天室/5.2.2 服务端】对比

使用==epoll模型==实现聊天室服务端，使用==父子进程==进行数据处理，并为所有客户socket连接的==读缓冲设计为一块共享内存==

算法流程：

- 1、声明、定义变量：
  - 共享内存相关变量
  - epoll句柄
  - 客户数据结构体：包含客户socket地址、客户连接套接字fd、处理这个客户的子进程pid、==父进程通信的管道pipefd[2]==
  - 客户数据结构体数组指针
  - 子进程数组（int）指针：<int, int>子进程pid和客户数据结构体数组索引（用户编号）的映射
  - 信号管道

- 2、父进程（main）

  - 将==lfd添加到epoll中==
  - 初始化客户数据结构体
  - 初始化子进程数组映射，每个子进程对应的用户编号为-1
  - 设置常见信号的处理函数，并将==信号管道读端添加到epoll中==
  - 初始化共享内存，这块内存作为所有客户socket连接的读缓冲区
  - 监听epoll模型：
    - 如果lfd发生读事件，说明有新客户：
      - （1）判断是否超过最大客户连接
      - （2）更新客户数据结构体数组的socket地址和客户连接套接字fd
      - （3）==创建双向管道，用于父子进程通信==
      - （4）fork子进程
        - pid==0：
          - 关闭父进程epoll句柄、关闭lfd、关闭客户数据结构体中的管道读端、关闭信号管道
          - 运行子进程程序
          - 解除内存映射
        - pid>0：
          - 关闭连接套接字cfd、关闭客户数据结构体中的管道写端
          - 将客户数据结构体中的==管道读端添加到epoll中==
          - 更新客户数据结构体数组的子进程pid
          - 更新子进程数组的用户编号（初始为0）
          - 更新当前用户数
    - 如果信号管道读端发生可读事件，说明有信号中断：
      - 获取信号管道读端信息
        - SIGCHLD：子进程终止（客户断开连接）。对子进程进行收尸，同时删除用户信息：根据子进程数组映射，获得需要删除的用户索引；从epoll中删除管道读端并关闭；更新客户数组结构体；更新子进程数组
        - SIGTERM/SIGINT：服务端中断。关闭服务端，杀死所有子进程
    - 如果发生其他可读事件，说明==父子进程通过管道进行通信==
      - 读取管道数据，该数据是子进程发送的客户编号
      - 将该客户编号（有数据要转发的客户编号）通过管道发送给其他子进程，子进程负责客户数据接收和转发
  - 关闭所有资源、清除内存

  

- 3、子进程

  - 每个子进程创建一个epoll模型，负责监听客户连接cfd、与父进程通信的管道
  - 每个子进程设置SIGTERM中断信号处理函数
  - 监听子进程epoll模型：
    - 如果是cfd且可读，则说明客户发送数据，需要接收：
      - 清空共享内存，将数据**读取**到共享内存中（注意，每个客户拥有相同大小的缓冲空间，借助用户编号来存储）
      - ret>0：通过管道发送用户编号给父进程，等待父进程回应
    - 如果管道可读，则说明父进程向子进程通知进行客户数据转发：
      - 读取管道数据，该数据是有数据要转发的客户编号
      - 将共享内存中对应的客户编号所在缓冲区**发送**给cfd



## 4 消息队列

在两个进程块之间传递二进制数据的简单有效方式

每个数据块有一个特定的类型，接收方可以根据类型来有选择地接收数据，而不一定像管道那样必须以先进先出的方式接收数据

- 创建一个消息队列/获取一个已有的消息队列

```c
int msgget(key_t key, int msgflg);
成功返回消息队列标识符；失败返回0并设置errno
```

如果创建成功，则内核的数据结构msqid_ds将被创建并初始化：

```c
struct msqid_ds
{ 
    struct ipc_perm msg_perm;/*消息队列的操作权限*/
    time_t msg_stime;/*最后一次调用msgsnd的时间*/
    time_t msg_rtime;/*最后一次调用msgrcv的时间*/
    time_t msg_ctime;/*最后一次被修改的时间*/
    unsigned long__msg_cbytes;/*消息队列中已有的字节数*/
    msgqnum_t msg_qnum;/*消息队列中已有的消息数*/
    msglen_t msg_qbytes;/*消息队列允许的最大字节数*/
    pid_t msg_lspid;/*最后执行msgsnd的进程的PID*/
    pid_t msg_lrpid;/*最后执行msgrcv的进程的PID*/
};
```

- 把一条消息添加到消息队列中：

```c
int msgsnd(int msqid, const void *msg_ptr, size_t msg_sz, int msgflg);
- msqid：消息队列标识符
- msg_ptr：准备发送的消息
- msg_sz：数据大小
- msgflg:控制msgsnd的行为；它通常仅支持IPC_NOWAIT标志，即以非阻塞的方式发送消
    
msg_ptr参数指向一个准备发送的消息，消息必须定义为如下类型：
struct msgbuf
{ 
    long mtype;/*消息类型*/
	char mtext[512];/*消息数据*/
}; 
```

- 从消息队列中获取消息：

```c
int msgrcv(int msqid, void *msg_ptr,size_t msg_sz,long int msgtype, int msgflg);
- msgtype：指定接收何种类型的消息。我们可以使用如下几种方式来指定消息类型：
	- 等于0。读取消息队列中的第一个消息
	- 大于0。读取消息队列中第一个类型为msgtype的消息（除非指定了标志MSG_EXCEPT）
	- 小于0。读取消息队列中第一个类型值比msgtype的绝对值小的消息
```

- 设置消息队列属性：

```c
int msgctl(int msqid, int command, struct msqid_ds *buf);
- msqid:是由msgget调用返回的共享内存标识符
- command:指定要执行的命令
```

