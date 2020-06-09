/* 函数描述：使用fread\fwrite实现命令行复制文件
 * */

#include<stdio.h>
#include<stdlib.h>
#define BUFSIZE 1024

int main(int argc, char **argv) // 命令行传参, argc表示参数个数
{
	FILE* fp1 = NULL;
	FILE* fp2 = NULL;
	char buf[BUFSIZE];
	int n;

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
	// fread返回成功读取到的对象个数
	while((n = fread(buf, 1, BUFSIZE, fp1)) > 0)
		fwrite(buf, 1, n, fp2);

	fclose(fp1);
	fclose(fp2);
	
	exit(0);
}

