#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
static int get_type(const char* fname)
{
	struct stat st;
	if(stat(fname, &st) < 0)
	{
		perror("stat()");
		exit(1);
	}
	int ch = 0;
	switch (st.st_mode & S_IFMT) {
           case S_IFBLK:  ch='b';      break;
           case S_IFCHR:  ch='c';      break;
           case S_IFDIR:  ch='d';    break;
           case S_IFIFO:  ch='p';    break;
	   case S_IFLNK:   ch='l';    break;
           case S_IFREG:  ch='-';       break;
           case S_IFSOCK: ch='s'; break;
           default:    ch='?';        break;
        }
	return ch;
}


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "Usage...");
		exit(1);
	}	
	printf("%c\n", get_type(argv[1]));
	exit(0);
}

