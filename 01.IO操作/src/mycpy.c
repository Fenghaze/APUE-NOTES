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



