/*传入通配符，使用glob获得匹配到的所有文件*/

#include<stdio.h>
#include<stdlib.h>
#include<glob.h>
#define PAT "/etc/a*.conf"


int main()
{
    int err = 0;
    glob_t globres;    
    err = glob(PAT, 0, NULL, &globres);
    if (err)
    {
        printf("Error code= %d\n", err);
        exit(1);   /* code */
    }
    for (int i = 0; i < globres.gl_pathc; i++)
    {
        puts(globres.gl_pathv[i]);   /* code */
    }
    globfree(&globres);
    exit(0);    
}