#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<time.h>
#include<signal.h>
static int loop = 1;

static void alrm_handler(int s)
{
    loop = 0;
}

int main()
{
    int64_t i = 0;
    alarm(5);
    signal(SIGALRM, alrm_handler);
    while (loop)
    {
        i ++;
    }
    printf("%ld\n", i);

/*    int64_t i = 0;
    time_t end;
    end = time(NULL)+5;
    while (time(NULL) <= end)
    {
        i++;  
    }
    printf("%ld\n", i);*/

    
    exit(0);
}