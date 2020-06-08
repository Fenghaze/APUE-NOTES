/*
 * 描述：计算文件中的有效字符个数
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









