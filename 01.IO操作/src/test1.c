#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#define BUFSIZE 1024
int cout(int line, char buf[])
{
	int n=0;
	int cout = 0;
	for(int i =0; i<strlen(buf); i++)	
	{	
		cout ++;
		if(n == line) break;
		if(buf[i] == '\n')
			n ++;
	}
	return cout;

}

int main(int argc, char** argv)
{
	char buf[BUFSIZE];
	int fd1 = 0;
	int fd2 = 0;
	int n = 0;
	int line = 0;
	int cout1 = 0;
	int cout2 = 0;
	int offset1 = 0;
	int offset2 = 0;
	fd1 = open(argv[1], O_RDONLY);
	
	fd2 = open(argv[1], O_RDWR);
	// 获取整个文件
	while(1)
	{
		n = read(fd1, buf, BUFSIZE);
		if(n <= 0) break;
	}
	cout1 = cout(3, buf);
	cout2 = cout(2, buf);
	// 定位到fd1的第3行
	offset1 = lseek(fd1, cout1, SEEK_SET);
        // 定位到fd2的第2行	
	offset2 = lseek(fd2, cout2, SEEK_SET);

	char buf1[BUFSIZE];
	int pos,ret;
	// 读取fd1写入到fd2
	while(1)
	{
		n = read(fd1, buf1, BUFSIZE);
		if(n<=0) break;
		pos = 0;
		while(n > 0)
		{
			ret = write(fd2, buf1+pos, n);
			if(ret < 0)
			{
				perror("write()");
				exit(1);
			}
			n-=ret;
			pos+=ret;
		}
	}

	close(fd2);
	close(fd1);
	exit(0);
}
