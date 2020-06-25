#ifndef PROTO_H__
#define PROTO_H__
#define NAMESIZE    11
#define RCVPORT     "1989"
#include <stdint.h>
struct msg_st
{
    uint8_t name[NAMESIZE];    // 数据类型
    uint32_t math;
    uint32_t chinese;
}__attribute__((packed));   // 地址不对齐

#endif