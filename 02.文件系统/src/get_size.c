
/*获取文件的大小*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>


static int get_size(const char* fname)
{
	int flag = 0;
	struct stat st;
	flag = stat(fname, &st);
	if(flag < 0)
	{
		perror("stat()");
		exit(1);
	}
	return st.st_size;
}

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Usage...");
		exit(1);
	}

	fprintf(stdout, "%d\n", get_size(argv[1]));
	exit(0);
}
