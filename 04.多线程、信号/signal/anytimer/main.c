#include<stdlib.h>
#include<stdio.h>


static void f1(void* p)
{
    printf("f1():%s\n", p);   
}
static void f2(void* p)
{
    printf("f2():%s\n", p);   
}

int main()
{
    puts("Begin!");



    puts("End!");

    while (1)
    {
        wirte(1, ".", 1);
        sleep(1);    /* code */
    }
    

    exit(0);
}
