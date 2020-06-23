#include<stdio.h>
#include<stdlib.h>
#include "proto.h"

int main()
{
    key_t key;
    int msgid;
    struct rbuf;
    long mtype;
    key = ftok(KEYPATH, KEYPROJ);
    if(key < 0)
    {
        perror("ftok()");
        exit(1);
    }

    msgid = msgget(key, IPC_CREAT|0600);
    if(msgid < 0)
    {
        perror("msgget()");
        exit(1);
    }

    msgrcv(msgid, );

    msgctl();


    exit(0);
}