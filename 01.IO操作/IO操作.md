# 1 标准IO和系统调用IO

- stdio标准IO

- sysio系统调用IO（文件IO）



二者的区别：

系统调用IO是内核提供给用户的一个IO接口，不同的系统，提供的系统调用IO就不一样，开发人员使用时会比较麻烦。

标准IO是依赖于系统调用IO实现的，这是在任何系统上都可以使用的。



在开发时，优先使用标准IO，**可移植性好**，**合并系统调用**。



例子：

比如C语言中的`fopen()`标准IO函数

它在Linux系统下依赖的系统调用IO函数是`open()`

它在Windows系统下依赖的系统调用IO函数是`openfile()`



##  1.1 steam的概念

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
| `fseek()`,`ftell()`,`rewind()` |   文件指针相关函数   |
|           `fflush()`           | 合并系统调用相关函数 |



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



**示例1：**

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

写入每一个字符，如果到达末尾，返回EOF



## 案例1：cp 命令的实现

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



## 案例2：计算文件的有效字符个数

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



### 2.3.2 fputs()



## 2.3 fread()、fwrite()

### 2.3.1 fread()



### 2.3.2 fwrite()