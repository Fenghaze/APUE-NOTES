# 1 标准IO和系统调用IO

- stdio标准IO

- sysio系统调用IO（文件IO）



## 1.1 二者的联系和区别

**联系：**

系统调用IO是内核提供给用户的一个IO接口，不同的系统，提供的系统调用IO就不一样，开发人员使用时会比较麻烦。

标准IO是**依赖于系统调用IO实现的**，这是在任何系统上都可以使用的。



在开发时，优先使用标准IO，**可移植性好**，**合并系统调用**。



例子：

比如C语言中的`fopen()`标准IO函数

它在Linux系统下依赖的系统调用IO函数是`open()`

它在Windows系统下依赖的系统调用IO函数是`openfile()`



**区别：**

标准IO具有缓冲机制，将数据放到缓冲区后，等到一定时间后再执行操作，吞吐量大

fflush()如果被调用，就是强制刷新缓冲区，不等到规定时间执行操作（合并系统调用，立即响应）



系统调用IO是实时处理的，每调用一次函数就立刻从user态切换到kernel态，响应速度快



==问题：如何使一个程序变快？==

从两个方面回答：吞吐量、响应速度



> **示例：标准IO和系统调用IO putchar_write.c**

```c
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main()
{
	putchar('a');
	write(1, "b", 1);

	putchar('a');
	write(1, "b", 1);

	putchar('a');
	write(1, "b", 1);
	
	exit(0);
}
```

最后的输出结果是：

```c
bbbaaa
```

由此可以看出，系统调用IO是实时的，而标准IO是将字符先存入缓冲区，再执行操作。



##  1.2 stream的概念

在系统中，文件是以流的形式出现，三种类型：

- stdin：标准输入流
- stdout：标准输出流
- stderr：标准错误流



# 2 stdio：标准IO

在stdio中，FILE数据类型==贯穿始终==

|             函数名             |         描述         |
| :----------------------------: | :------------------: |
|      `fopen()`,`fclose()`      |    打开\关闭文件     |
|     `fgetc()` , `fputc()`      |    字符读取\写入     |
|      `fgets()`，`fputs()`      |   字符串读取\写入    |
|     `fread()`，`fwrite()`      |   二进制读取\写入    |
|           `printf()`           |       打印输出       |
|           `scanf()`            |       键盘输入       |
| `fseek()`,`ftell()`,`rewind()` | 文件位置指针相关函数 |
|           `fflush()`           | 合并系统调用相关函数 |
|          `getline()`           |     读取一行字符     |



## 2.1 fopen()、fclose()

### 2.1.1 fopen()

（1）`FILE *fopen(const char *pathname, const char *mode);`

- pathname：文件路径
- mode：操作文件的方式（r, r+, w, w+, a, a+，如果文件是二进制，加上b）

如果成功，返回一个**FILE指针**；否则返回一个NULL指针和一个errno

==返回的FILE指针，是指向堆区的。==



### 2.1.2 fclose()

`int fclose(FILE *stream);`

- stream：FILE指针



> **示例1：fopen.c**

```c
#include<stdlib.h>
#include<errno.h> // 使用errno时，需要包含头文件
#include<stdio.h>

int main()
{
	FILE* fp;
	fp = fopen("hello", "r");
	
	if(fp == NULL)
	{
		//fprintf(stderr, "fopen() failed! errno = %d\n", errno);
		//perror("fopen()");
        fprintf(stderr, "fopen(): %s\n", sterror(errno)); // 包含头文件 string.h
        exit(1);
	}
	puts("ok!");
    fclose(fp);
	exit(0);
}
```

==补充，报错函数==

- `void perror(const char *s)`：自动将errno转换为错误信息打印

- `char *strerror(int errnum)`：将errno转换为错误信息打印



## 2.2 fgetc()、fputc()

### 2.2.1 fgetc()

`int fgetc(FILE *stream);`

- stream：标准输入stdin流的指针

读取每一个字符，如果读到末尾，返回EOF



### 2.2.2 fputc()

` int fputc(int c, FILE *stream);`

- c：输出字符
- stream：标准输出stdout流的指针

输出每一个字符，如果到达末尾，返回EOF



## 案例1：cp 命令的实现 mycpy.c

```c
/* 函数描述：使用命令行复制文件
 * ./mycpy file1   file2
 * argv[0] argv[1] argv[2]
 * */

#include<stdio.h>
#include<stdlib.h>

int main(int argc, char **argv) // 命令行传参, argc表示参数个数
{
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	int ch;

	// 对于命令行参数，需要进行预先判断
	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
		exit(1);
	}
	
	// 读文件
	fp1 = fopen(argv[1], "r");
	if(fp1 == NULL)
	{
		perror("fopen()");
		exit(1);
	}	

	// 写文件
	fp2 = fopen(argv[2], "w");
	if(fp2 == NULL)
	{
		perror("fopen()");
		exit(1);
	}
	
	while(1)
	{
		ch = fgetc(fp1);
		if(ch == EOF)
			break;
		fputc(ch, fp2);
	}

	fclose(fp1);
	fclose(fp2);
	
	exit(0);
}
```



## 案例2：计算文件的有效字符个数 cout_num.c

```c
/* 描述：计算文件中的有效字符个数
 * */
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char** argv)
{

	FILE* fp = NULL;
	int cout = 0;
	if (argc < 2)
	{
		fprintf(stderr, "Usage: %s <file>\n", argv[0]);
		exit(1);
	}
	
	// 读文件流指针
	fp = fopen(argv[1], "r");
    // 判断是否打开成功
	if(fp == NULL)
	{
		perror("fopen()");
		exit(1);
	}
	
	while(fgetc(fp) != EOF)
		cout ++;

	fclose(fp);
	fprintf(stdout, "count = %d\n", cout);
	exit(0);
}
```



## 2.3 fgets()、fputs()

### 2.3.1 fgets()

`char *fgets(char *s, int size, FILE *stream);` 

将标准输入流读取到缓冲区s，缓冲区大小为size

- s：存放读取的内容的缓冲区
- size：要读取的字符串长度
- stream：要读取的标准输入流stdin

读取每一个字符，当读到size-1或者换行符（`\n`）时停止，'\0'表示读取结束

如果读取成功，返回s；否则返回一个空指针



### 2.3.2 fputs()

`int fputs(const char *s, FILE *stream);` 

将缓冲区s的字符，输出到标准输出流中

- s：缓冲区的指针
- stream：标准输出流stdout

输出每一个字符，如果到达末尾，返回EOF



>  **示例：重构案例1的代码 mycpy2.c** 

```c
/* 函数描述：使用fputs\fgets实现命令行复制文件
 * */
#include<stdio.h>
#include<stdlib.h>
#define BUFSIZE 1024

int main(int argc, char **argv)
{
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	char buf[BUFSIZE];
	...
	while(fgets(buf, BUFSIZE, fp1) != NULL)
		fputs(buf, fp2);
	...
}
```



## 2.4 fread()、fwrite()

### 2.4.1 fread()

`size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);` 

从stream读取字符存放到ptr缓冲区

- ptr：存放读取的内容的缓冲区，缓冲区大小为 size*nmemb
- size：一个对象的大小
- nmemb：读取的对象个数
- stream：标准输入流stdin

返回成功读取到的对象的个数；否则返回0



nmemb和size的理解：

假设文件有15个字符，nmemb=5

相当于把文件的字符划分成 5 个对象，每个对象的 size 为 3，故总的大小为 nmemb*size



### 2.4.2 fwrite()

`size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);` 

获取缓冲区的字符，输出到标准输出流

- ptr：缓冲区的指针
- stream：标准输出流stdout

返回成功写入的对象的个数；否则返回0



> **示例：重构案例1的代码 mycpy3.c**

```c
/* 函数描述：使用fread\fwrite实现命令行复制文件
 * */
#include<stdio.h>
#include<stdlib.h>
#define BUFSIZE 1024

int main(int argc, char **argv)
{
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	char buf[BUFSIZE];
    int n;
	...
	while((n = fread(buf, 1, BUFSIZE, fp1)) > 0)
		fwrite(buf, 1, n, fp2);
	...
}
```



## 2.5 fprintf()、fscanf()

### 2.5.1 fprintf()

`int fprintf(FILE *stream, const char *format, ...);`

- stream：指定流（stdout, stderr）
- format：输出字符串



`int sprintf(char *str, const char *format, ...);`

将多种格式的输出项打包成一个字符串



### 2.5.2 fscanf()

`int fscanf(FILE *stream, const char *format, ...);`

- stream：指定流（stdin, stderr）



## 2.6 fseeko()、ftello()

### 2.6.1 fseeko()

定位，设置文件流的光标位置（文件位置指针），不进行IO操作

`int fseeko(FILE *stream, off_t offset, int whence);`

- stream：待定位的文件流
- offset：偏移量（正负表示前后），与fseek不同，此处的偏移量可以改为64位，详情查看man手册
- whence：指定偏移位置（SEEK_SET文件开头，SEEK_CUR当前位置，SEEK_END文件末尾）



`void rewind(FILE *stream);` 

将光标定位到文件首



### 2.6.2 ftello()

**获得文件流的当前光标位置（文件位置指针）**，不进行IO操作

`off_t ftello(FILE *stream);`

- stream：文件流



> **示例：重构案例2的代码 cout_num2.c**

```c
/*
 * 描述：使用文件位置指针计算文件中的有效字符个数
 * */
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char** argv)
{
	FILE* fp = NULL;
	int cout = 0;
	...
	fseek(fp, 0, SEEK_END);
	cout = ftell(fp);
	fprintf(stdout, "count = %d\n", cout);
	exit(0);
}
```



## 2.7 ==fflush() 刷新缓冲==

==缓冲区的作用：合并系统调用==

缓冲模式：

- 行缓冲：换行的时候刷新、缓冲区满了时候刷新、强制刷新（标准输出）
- 全缓冲：缓冲区满了时候刷新，强制刷新（默认）
- 无缓冲：如stderr，需要立即输出的内容



刷新指定流

`int fflush(FILE *stream);`

- stream：文件流



## 2.8 getline()

从文件流中读取一行内容

`ssize_t getline(char **lineptr, size_t *n, FILE *stream);`

==**getline的实现是基于内存动态申请实现的**==



> **示例：获取文件中每行个数 getline.c**

```c
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char** argv)
{
	FILE * fp =NULL;
	char * linebuf = NULL;
	size_t linesize = 0;

	if(argc < 2)
	{
		fprintf(stderr, "Usage...\n");
		exit(1);
	}

	fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		perror("fopen()");
		exit(1);
	}	

	while(getline(&linebuf, &linesize, fp) >= 0)
	{
		printf("%d\n", strlen(linebuf));
	}
	fclose(fp);
	exit(0);
}
```



## 2.9 临时文件

需要考虑的问题：

- 1、如何不冲突
- 2、及时销毁



有两个函数可以实现：

1、为一个临时文件创建一个名字：`char *tmpnam(char *s);`

2、创建一个**匿名**的临时文件，返回一个FILE指针：`FILE *tmpfile(void)`

