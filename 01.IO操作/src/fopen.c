#include<stdlib.h>
#include<errno.h>
#include<stdio.h>

int main()
{
	FILE* fp;
	fp = fopen("hello", "r");
	
	if(fp == NULL)
	{
		fprintf(stderr, "fopen() failed! errno = %d\n", errno);
		exit(1);
	}
	puts("ok!");
	exit(0);
}





