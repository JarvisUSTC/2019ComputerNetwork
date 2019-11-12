#ifndef CLIENT_REALIABLE_H
#define CLIENT_REALIABLE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>

#define BUF_SIZE 512
#define LOAD_SIZE 57
#define SEQ_MAX 256
#define WIN_SIZE 8
typedef uint8_t tcp_seq;

typedef struct tcphdr {
    uint8_t btcp_sport;
    uint8_t btcp_dport;
    tcp_seq btcp_seq; //序列号 
    tcp_seq btcp_ack; //确认号
    uint8_t data_off; //偏移地址
    uint8_t win_size; //窗口大小 用来流量控制 N
    uint8_t flag; //是否为重传包   
} BTcpHeader;//size:7 bytes

typedef struct package
{
    BTcpHeader header;//7bytes
    char payload[LOAD_SIZE];//我感觉应该判断一下长度 可能设置一个64b的缓存 
}package;//fix the size:71

package* data_package(tcp_seq btcp_seq,tcp_seq btcp_ack,uint8_t data_off,uint8_t win_size,uint8_t flag,char* payload);


int expectedseqnum = 0;

#endif