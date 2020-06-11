# 1 下载源码

源码地址：http://www.apuebook.com

```shell
tar -zxv -f src.3e.tar.gz # 解压缩
cd apue.3e/
sudo apt-get install libbsd-dev # 安装依赖
make # make编译
```

# 2 make报错解决办法

```shell
cc -ansi -I../include -Wall -DLINUX -D_GNU_SOURCE  devrdev.c -o devrdev  -L../lib -lapue 
devrdev.c: In function ‘main’:
devrdev.c:19:25: warning: implicit declaration of function ‘major’ [-Wimplicit-function-declaration]
   19 |   printf("dev = %d/%d", major(buf.st_dev),  minor(buf.st_dev));
      |                         ^~~~~
devrdev.c:19:45: warning: implicit declaration of function ‘minor’ [-Wimplicit-function-declaration]
   19 |   printf("dev = %d/%d", major(buf.st_dev),  minor(buf.st_dev));
      |                                             ^~~~~
/usr/bin/ld: /tmp/ccftLgS4.o: in function `main':
devrdev.c:(.text+0xc5): undefined reference to `minor'
/usr/bin/ld: devrdev.c:(.text+0xdb): undefined reference to `major'
/usr/bin/ld: devrdev.c:(.text+0x128): undefined reference to `minor'
/usr/bin/ld: devrdev.c:(.text+0x13e): undefined reference to `major'
collect2: error: ld returned 1 exit status
make[1]: *** [Makefile:18: devrdev] Error 1
make[1]: Leaving directory '/home/eric/Downloads/apue.3e/filedir'
make: *** [Makefile:6: all] Error 1
```



## 2.1 修改 Makefile 文件

删掉**DIRS**变量中的 `filedir`

```
DIRS = lib intro sockets advio daemons datafiles db environ \
	fileio filedir ipc1 ipc2 proc pty relation signals standards \
	stdio termios threadctl threads printer exercises


DIRS = lib intro sockets advio daemons datafiles db environ \
	fileio ipc1 ipc2 proc pty relation signals standards \
	stdio termios threadctl threads printer exercises
```



## 2.2 修改 ./stdio/buf.c 文件

修改buf.c文件最后的三个函数

```c
#ifdef _LP64
#define _flag __pad[4]
#define _ptr __pad[1]
#define _base __pad[2]
#endif

int
is_unbuffered(FILE *fp)
{
	return(fp->_flags & _IONBF);	
}

int
is_linebuffered(FILE *fp)
{
	return(fp->_flags & _IOLBF);
}

int
buffer_size(FILE *fp)
{
#ifdef _LP64
	return(fp->_IO_buf_end - fp->_IO_buf_base);
#else
	return(BUFSIZ);	/* just a guess */
#endif
}
```



# 3 复制静态链接库

```shell
sudo cp include/apue.h /usr/include/
sudo cp lib/error.c /usr/include/ 
sudo cp lib/libapue.a /usr/lib
```



# 4 若出现err_sys等函数在编译时找不到的问题

添加这行代码到 /include/apue.h 文件里：

```c
#include "error.c"
```