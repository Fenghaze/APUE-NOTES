/* 函数描述：使用系统调用IO实现命令行复制文件*/

#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#define BUFSIZE 1024
#include <sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
int main(int argc, char **argv) // 命令行传参, argc表示参数个数
{

	int fd1 = 0;
	int fd2 = 1;
	int n = 0; //成功读取到的字符个数
	int ret = 0; //成功写入的字符个数
	int pos = 0;
	char buf[BUFSIZE];	
	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s <file1> <file2>\n", argv[0]);
		exit(1);
	}
	

	// 读文件
	fd1 = open(argv[1], O_RDONLY);
	if(fd1 < 0)
	{
		perror("open()");
		exit(1);
	}
	

	// 写文件,如果不存在，则创建，并赋予权限 0600
	fd2 = open(argv[2], O_WRONLY|O_CREAT,O_TRUNC, 0600);
	if(fd2 < 0)
	{
		close(fd1);
		perror("open()");
		exit(1);
	}
	
	while(1)
	{
		n = read(fd1, buf, BUFSIZE);
		if(n < 0)
		{
			perror("read()");
			break;
		}
		if(n == 0)
			break;
		pos = 0;
		while(n > 0)
		{
			ret = write(fd2, buf+pos, n);
			if(ret < 0)
			{
				perror("write()");
				exit(1);
			}
			n -= ret;
			pos += ret;
		}
	}

	close(fd2);
	close(fd1);
	
	exit(0);
}

