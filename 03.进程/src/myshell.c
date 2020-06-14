#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<glob.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#define DLIMS " \t\n"
struct cmd_st
{
    glob_t globres;   /* data */
};

static void prompt()
{
    printf("myshell-0.1$ ");
}
static void parse(char* line, struct cmd_st *res)
{
    char* tok;
    glob_t globres;
    int i = 0;
    while (1)
    {
        tok = strsep(&line, DLIMS); // 分隔命令行
        if(tok == NULL)
            break;
        if(tok[0] == '\0')
            continue;
        // 使用glob解析命令行，并保存到res.globres中
        glob(tok, GLOB_NOCHECK|GLOB_APPEND*i, NULL, &res->globres);
        i =1;
    }
    
}
int main()
{
    int pid;
    char* linebuf = NULL;
    size_t linebuf_size = 0;
    struct cmd_st cmd;
    while(1)
    {
        prompt();

        if(getline(&linebuf, &linebuf_size, stdin) < 0)
            break;

        parse(linebuf, &cmd);        
    
        if(0) // 如果是内部命令
        {/*do something*/}
        else    // 如果是外部命令
        {
            pid = fork();
            if(pid < 0)
            {
                perror("fork()");
                exit(1);
            }   
            if(pid == 0)
            {
                execvp(cmd.globres.gl_pathv[0], cmd.globres.gl_pathv);
                perror("execvp()");
                exit(1);
            }/* code */
            else
            {
                wait(NULL);
            }
        }

    }
    pid = fork();


    exit(0);
}