#include<stdio.h>
#include<stdlib.h>
#include<s

static int64_t mydu(const char* path)
{
    
    // 判断path是否是目录文件
    

    // path 为非目录文件
    
    
    glob();
    return 

    // path 为目录文件
    glob();

}


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage...\n");
        exit(1);   /* code */
    }
    int size = mydu(argv[1]);
    printf("%lld\n", size);
    exit(0);
}