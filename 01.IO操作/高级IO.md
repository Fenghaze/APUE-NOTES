# 1 非阻塞IO

两个设备之间的数据交换称为数据中继



假设有这样一个数据交换的任务：

读设备A->将读取到的内容写入到设备B->读设备B->将读取到的内容写入到设备A



对于单线程（单进程）来说

阻塞IO：如果设备A没有内容，就会一直阻塞在读设备A的位置，由于设备A被阻塞且只有一个线程在工作，那么后续任务无法执行下去

非阻塞IO：**如果设备A没有内容，那么就会放弃这一步，去执行下面的操作，不会卡死在某一个环节**



对于多线程（多进程）来说：将任务进行划分，一个线程执行读设备A，写到设备B的任务；一个线程执行读设备B，写到设备A的任务

阻塞IO：即便有一个任务阻塞了，也不会影响到整体的任务循环

## 1.1 阻塞IO，非阻塞IO（NIO），异步IO（AIO）

阻塞IO：会一直等到有数据才执行下一步

非阻塞IO：使用非阻塞IO时，如果有数据则立即返回结果，否则返回错误码；不会像阻塞IO一直停留在这一步

异步IO：调用异步IO后，会执行下一步，当有数据时，内核会发起通知



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

服务端的`accept`和`read`默认都是阻塞的

来一个客户端发起`connect`请求时，就创建一个进程或线程去处理，这种方式实现并发；然而大多数时间，当客户端没有发送数据时，服务端开启的进程或线程就没有任务，此时都处于休眠状态（盲等状态），这样非常浪费内存资源



有一种解决办法是：非阻塞忙轮询

就是让一个进程不停的访问每一个套接字（监听套接字和已连接套接字），查看是否有连接或数据到达，但是这种方法非常浪费CPU



最优的办法是IO多路复用：

![](./src/IO多路复用.png)

==设置一个进程用来监听多个文件描述符的读写缓冲区的变化，只有当文件描述符读写状态发生改变时，才去推动状态机，执行任务，而不是盲目等待==

监听到的文件描述符分两类,一个是lfd（监听套接字），一类是cfd（已连接套接字）

```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);


int poll(struct pollfd *fds, nfds_t nfds, int timeout);


epoll; // 是poll在Linux封装的方言
```



==大致步骤：==

- 1、布置监视任务
- 2、选择监视函数（`select`，`poll`，`epoll`）进行监视
- 3、根据监视的情况来执行相应的操作



## 3.1 select

`int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);`

成功，返回监听集合中状态发生变化的文件描述符个数，内核将自动修改文件描述符集合来通知应用程序哪些文件描述符已经就绪；如果失败，返回-1和errno

- nfds：监视的最大文件描述符+1
- readfds：文件描述符是否可读的集合（监听集合中的文件描述符的读事件）
- writefds：文件描述符是否可写的集合
- exceptfds：异常文件描述符的集合
- timeout：超时设置。NULL（select阻塞，永久监听），0（不等待，立即返回）
- ==fd_set==结构体包含一个整型数组，数组长度由FD_SETSIZE指定，该数组中的每个元素的每一位（bit）标记一个文件描述符，表示==位图==

使用下面的一系列宏来控制fd_set结构体中的位：

```c
void FD_ZERO(fd_set *set);			//将集合清空

void FD_CLR(int fd, fd_set *set);	//将一个fd从set中清除

void FD_SET(int fd, fd_set *set);	//将一个fd添加到set中

int  FD_ISSET(int fd, fd_set *set);	//判断fd是否在集合中
```



==当有文件描述符可读/可写的时候，监听集合就发生变化==



**问题1：当有新的客户端要请求连接服务端时，需要监听哪个`socket`，加入哪个集合？**

对于服务端而言，新的客户端加入时，监听的是服务端的`socket`，加入readfds可读集合

**问题2：客户端向服务端发送数据，这对于服务端中使用`select`监听的socket而言，是加入哪个集合？**

客户端使用`send`或`write`对socket写入数据后（相当于发送了数据）；于服务端而言，连接到的socket中就有内容了，于是服务端监听的socket==可读==，即加入readfds可读集合

**问题:3：如何将需要监听的文件描述符添加到各个集合中？**

先调用`FD_ZERO`，再`FD_SET`，此时集合中的文件描述符对应的==位图置1==

**问题4：对于成功返回的文件描述符个数，如何区分哪个fd来自与哪个集合？**

当内核监听到有文件描述符的状态发生变化时（如readfds初始监听3个文件描述符，对应位图为111；有一个读事件发生，readfds位图变为001），内核自动将变化后的集合返回给用户

用户可以使用for循环，调用`FD_ISSET`来判断集合中是否存在fd，从而判断这个fd的状态是否发生变化

**问题5：文件描述符可读情况？**

- socket内核接收缓存区中的字节数大于或等于其低水位标记SO_RCVLOWAT。此时我们可以无阻塞
  地读该socket，并且读操作返回的字节数大于0。
- socket通信的对方关闭连接。此时对该socket的读操作将返回0。
- 监听socket上==有新的连接请求==。
- 连接socket接收到数据
- socket上有未处理的错误。此时我们可以使用getsockopt来读取和清除该错误

**问题6：文件描述符可写情况？**

- socket内核发送缓存区中的可用字节数大于或等于其低水位标记SO_SNDLOWAT。此时我们可以无
  阻塞地写该socket，并且写操作返回的字节数大于0。
- socket的写操作被关闭。对写操作被关闭的socket执行写操作将触发一个SIGPIPE信号。
- 客户端socket使用非阻塞connect连接成功或者失败（超时）之后。
- socket上有未处理的错误。此时我们可以使用getsockopt来读取和清除该错误。

**问题7：文件描述符异常情况？**

socket上接收到==带外数据==



**补充：当有监听事件发生时，集合都会发生变化，内核会将变化后的集合复制到用户空间，而下一次开始新的监听时，仍然是要监听原来的事件，此时可以做一个备份集合，这个备份集合保存的是一直要监听的文件描述符**



> 例子：src/nonblock/select.c

算法流程：

```c
1、创建套接字
2、绑定
3、监听
while(1){  
    （1）设置要监听的集合
    （2）select()//监听
    （3）判断集合变化：
    	如果是lfd变化,提取新的连接,并且将新的套接字加入监听的集合
    	如果是cfd变化,直接读取,处理客户端请求
}
```



### select优化

在判断cfd的状态是否在监听集合中，方法是遍历所有已连接套接字【lfd+1， maxfd】，一个一个遍历是否在可读集合中

==在Linux中，默认监听的最大描述符个数为1024==



**一种情况是**，监听【3，1023】的文件描述符，在运行过程中，一些客户端关闭了端口，此时仍然需要循环多次来判断处理，效率很低，需要优化；

【优化方法】

- C

==自定义一个监听数组，该数组初始化为-1，当有新的客户端加入时，保存socket到监听数组，值变为cfd==

在判断所有已连接socket时，从监听数组开始遍历，值为-1的直接跳过

- C++

使用vector代替数组



**另一种情况是**，监听【3，1023】的文件描述符，其中只有少部分的socket经常发来消息，而其他的socket偶尔发送消息且并没有关闭，出现了资源浪费，==无法优化（select缺陷：大量并发，少量活跃）==



**优点:**跨平台、并发量高、效率高、消耗资源少、消耗cpu也少

**缺点: **

- 有最大文件描述符的个数限制 FD_SETSIZE = 1024 

- 每一次重新监听都需要再次设置需要监听集合,集合从用户态拷贝至内核态消耗资源
- 监听到文件描述符变化之后,需要用户自己遍历集合才知道具体哪个文件描述符变化了  
- 大量并发，少数活跃，select的效率低



## 3.2 poll

poll系统调用和select类似，也是在指定时间内轮询一定数量的文件描述符

`int poll(struct pollfd *fds, nfds_t nfds, int timeout);`

- fds：存放了文件描述符及其事件的结构体数组的起始地址
- nfds：数组中文件描述符个数
- timeout：超时设置

成功，返回监听集合中状态发生变化的文件描述符个数，内核将自动修改revents来通知应用程序哪些文件描述符已经就绪；如果失败，返回-1和errno

```c
struct pollfd {
    int   fd;      /* file descriptor */
    short events;  /* 要监听的事件按位或：POLLIN/POLLOUT/POLLERR */
    short revents; /* 当监听到某事件后，内核会自动修改，通知应用程序fd上实际发生了哪些事件,初始化为0 */
};
触发情况：
- POLLIN：可读（普通数据和优先数据）
    - POLLRDNORM：普通数据可读
    - POLLPRI：优先级数据可读，如TCP带外数据
- POLLOUT：可写（普通数据和优先数据）
    - POLLWRNORM：普通数据可写
    - POLLWRBAND：优先级数据可写
- POLLRDHUP：客户端关闭连接/客户端关闭写操作
- POLLHUP：挂起，如管道写端被关闭后，读端fd上将收到POLLHUP事件
- POLLERR：错误
```

> 例子：poll.c



优点：没有监听上限；监听、返回集合分离；



## 3.3 ==epoll==

epoll是Liunx的方言，是select/poll的增强版本，使用一组函数来完成任务。**epoll把用户关心的文件描述符上的事件放在内核里的一个事件表中，而无须像select和poll那样每次调用都要重复传入文件描述符集或事件集**

epoll的存储结构是一棵**红黑树**

优点：

- 没有文件描述符个数限制
- 已经添加到红黑树上的cfd，如果状态变化，会自动存储到监听事件数组events中；当开始新一轮监听时，不需要重新将cfd添加到红黑树中
- 监听到文件描述符变化后，返回的是已经变化的文件描述符，不需要像select对文件描述符进行判断其是否在可读集合中

```c
// 1、创建epoll句柄（红黑树）
int epoll_create(int size); 
- size：监听的文件描述符个数
成功，返回一个epfd（epfd是一棵红黑树的根节点，有size个文件描述符作为结点）

// 2、设置epoll：在红黑树上添加需要监听的文件描述符
int  epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
- op：对红黑树中的结点进行的操作（增加/删除/修改文件描述符结点）
    -  EPOLL_CTL_ADD:添加结点
    -  EPOLL_CTL_MOD:修改结点
    -  EPOLL_CTL_DEL:删除结点
- event：监听事件结构体的地址（事件+数据）
    -  EPOLLIN：读事件
    -  EPOLLOUT：写事件
    -  EPOLLERR：错误事件
成功返回0，失败返回-1并设置errno

// 3、监听等待，相当于select()
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
- epfd：文件描述符，红黑树根节点
- events：监听事件结构体数组的首地址（数组需要提前定义）
- maxevents：数组元素个数，最多监听多少个事件
- timeout：超时等待
成功，返回监听到的文件描述符个数，内核空间会将监听到的就绪事件复制一份存放到用户空间定义的第二个参数events结构体数组中，用户只需要对这个结构体数组进行操作即可；失败返回-1并设置errno

// 联合体
typedef union epoll_data {
    void        *ptr;	// 应用于epoll反应堆模型
    int          fd;	// 监听的文件描述符
    uint32_t     u32;
    uint64_t     u64;
} epoll_data_t;

struct epoll_event {
    uint32_t     events;      /* 位图，监听事件：EPOLLIN/OUT/ERR */
    epoll_data_t data;        /* User data variable */
};
```



算法流程：

```c
1、创建套接字
2、绑定
3、监听
4、创建红黑树根节点
5、将lfd添加到红黑树，设置为可读事件
while(1){  
    （1）epoll_wait()//监听红黑树
    （3）判断集合变化：
    	如果是lfd变化,提取新的连接,并且将新的套接字加入到红黑树
    	如果是cfd变化,直接读取,处理客户端请求
}
```

==注意：==

epoll_events中的events成员是一个位图，当监听到事件后，文件描述符的事件位图发生了变化，需要进行位与（`&`）来判断事件是否发生变化



### epoll和poll的区别

```c++
/*如何索引poll返回的就绪文件描述符*/
int ret=poll(fds,MAX_EVENT_NUMBER,-1);
/*必须遍历所有已注册文件描述符并找到其中的就绪者（当然，可以利用ret来稍做优化）*/
for(int i=0;i＜MAX_EVENT_NUMBER;++i)
{
	if(fds[i].revents＆POLLIN)/*判断第i个文件描述符是否就绪*/
    {
    	int sockfd=fds[i].fd;
    	/*处理sockfd*/
    }
}

/*如何索引epoll返回的就绪文件描述符*/
int ret=epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
/*仅遍历就绪的ret个文件描述符*/
for(int i=0;i＜ret;i++)
{
    int sockfd=events[i].data.fd;
    /*sockfd肯定就绪，直接处理*/
}
```

- poll模型：所有文件描述符及其事件存放在一个结构体数组fds中，当poll函数返回成功时，需要遍历fds先判断每个fd的可读可写情况，再进行读/写操作
- epoll模型：文件描述符及其事件是一个红黑树节点，用户定义了一个events数组，当epoll_wait函数返回成功时，红黑树上监听的节点事件会自动发生变化，同时内核空间会将监听到的就绪事件复制一份存放到events结构体数组中，遍历events数组进行读/写操作



### epoll触发模式

不同的触发模式会影响epoll_wait函数的执行次数

- 水平触发（电平一直为0或1就触发）：EPOLLLT（默认）
  - **读缓冲区只要有数据就会被触发；写缓冲区只要可写就会被触发**
  - 比如远端发送10个字节的数据，服务端一次处理5个字节，缓冲区还剩5个字节，数据需要读取两次才能将缓冲区读完，那么`epoll_wait()`就会一直触发两次
- ==边沿触发（电平由0变1或由1变0就触发）==：EPOLLET
  - **读缓冲区只要有数据到达就会被触发；写缓冲区只要数据被发送了就会被触发**
  - 比如远端发送10个字节的数据，服务端一次处理5个字节，缓冲区还剩5个字节，但是`epoll_wait()`只会触发一次，如果不使用循环读完，那么剩下的字节就无法读取；只有当**新的数据**发送过来时，才会再次触发，继续读剩下的字节
  - ==写法：==事件监听时，使用`EPOLLIN|EPOLLET`
- ==epoll非阻塞==：`fcntl() `

```c++
// 将新客户端cfd设置为非阻塞，read为非阻塞
int flag = fcntl(cfd, F_GETFL);
flag |= O_NONBLOCK;
fcntl(cfd, F_SETFL, flag);
```



==注意：==

使用边沿触发时，缓冲区可能需要使用while循环读取多次才能将缓冲区读完，当读完缓冲区，`read()`陷入阻塞状态，需要提前将cfd设置为非阻塞，并跳出循环

**当read的返回值为-1,并且errno的值被设置为EAGAIN时,代表缓冲区被读取干净**

可见，==ET模式在很大程度上降低了同一个epoll事件被重复触发的次数，因此效率要比LT模式高，在实际工作中，通常采用边沿触发+epoll非阻塞==



### epoll 难点

==**EPOLLIN与EPOLLOUT的理解？**==

EPOLLIN：当一个连接过来，或者一个数据发送过来了，那么EPOLLIN事件就触发

EPOLLOUT：当写缓冲区可写，就可以触发

==**水平触发（LT）时的EPOLLIN和EPOLLOUT**==

EPOLLIN：读缓冲区**只要有数据就会一直触发**

EPOLLOUT：写缓冲区**只要有空间就会一直触发**

==**边沿触发（ET）时的EPOLLIN和EPOLLOUT**==

ET模式称为边缘触发模式，顾名思义，**不到边缘情况，是死都不会触发的。**

**EPOLLOUT事件：**
EPOLLOUT事件只有在连接时触发一次，表示可写，其他时候想要触发，那你要先准备好下面条件：

- 1.某次write，写满了发送缓冲区，返回错误码为EAGAIN。
- 2.对端读取了一些数据，又重新可写了，此时会触发EPOLLOUT。

简单地说：EPOLLOUT事件只有在不可写到可写的转变时刻，才会触发一次，所以叫边缘触发，这叫法没错的！

其实，如果想==强制触发==一次，也是有办法的，==直接调用`epoll_ctl`重新设置一下event就可以了==，event跟原来的设置一模一样都行（但必须包含EPOLLOUT），==关键是重新设置，就会马上触发一次EPOLLOUT事件==

**EPOLLIN事件：**
EPOLLIN事件则只有当对端有数据写入时才会触发，所以触发一次后需要不断读取所有数据直到读完**EAGAIN**为止。否则剩下的数据只有在下次对端有写入时才能一起取出来了。
现在明白为什么说epoll必须要求异步socket了吧？如果同步socket，而且要求读完所有数据，那么最终就会在堵死在阻塞里。



### ==reactor反应堆模型（libevent核心思想）==

libevent库：跨平台，使用了大量的回调函数，实现了高并发

反应堆是指反应快。。。



反应堆模型与普通epoll模型的区别：

- （1）反应堆模型增加了对客户读cfd的**可写事件**的监听
- （2）使用了**回调函数**

```c
// ptr指向这个事件驱动结构体
struct xxevent
{
    int fd;		// 要监听的文件描述符
    int events;	// 对应的监听事件
    void *arg;	// 泛型参数
    void (*call_back)(int fd, int events, void *arg);	// 回调函数
    int status;	// 是否在监听：1表示在红黑树上，0表示不在红黑树上
    char buf[BUFSIZE];
    int len;
    long last_active;	// 记录每次fd加入红黑树 efd 的时间，类似超时设置
}
```



算法步骤：

```shell
1、定义事件驱动结构体（epoll_event.data.ptr指向这个结构体）
2、定义全局红黑树根，方便使用
3、定义事件函数：
	eventadd()：初始化事件驱动结构体，添加结点到红黑树
	eventset()：修改事件驱动结构体，修改结点
	eventdel()：重置事件驱动结构体，删除结点
4、定义回调函数：
	initAccept()：接收新客户端cfd，调用eventadd()将cfd添加到红黑树，设置监听事件为EPOLLIN，回调函数为recvData()
	recvData()：接收已连接cfd发来的消息，将读取到的字符及字符数存放在事件驱动结构体中；当字符数>0时，调用eventset()修改cfd，监听事件改为EPOLLOUT，回调函数为sendData()；当字符数==0时，关闭cfd，调用eventdel()删除cfd
	sendData()：发送字符给cfd；调用eventset()修改cfd，监听事件改为EPOLLIN，回调函数为recvData()，表示继续监听cfd的读事件
	
5、main函数：
	创建lfd，配置、绑定、监听lfd
	定义事件数组
	调用eventadd()将lfd添加到红黑树，回调函数为initAccept()
	while(1)
	{
		n = epoll_wait();
		for循环处理事件数组中的事件，调用回调函数
	}
```



### ==epoll+线程池==

线程的工作是处理每个客户端的请求

main线程来监听lfd，设计一个线程池来处理任务



> 示例：./epoll_reactor/reactor.c


