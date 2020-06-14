#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main()
{
    puts("Begin!");
    int flag;

    execl("/bin/date", "date", "+%s", NULL);
    perror("execl()");
    exit(1);    


    puts("End!");
    exit(0);

}