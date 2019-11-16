#ifndef SERVER_REALIABLE_H
#define SERVER_REALIABLE_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <signal.h>

#define TIME_INTERVAL 100000

struct itimerval oldtv;


#define BUF_SIZE 512
#define LOAD_SIZE 64
#define WIN_SIZE 8
#define SEQ_MAX 256
#define FULL 1
#define EMPTY 10
typedef uint8_t tcp_seq;


int result_send;
int result_recv;

typedef struct input_send
{
    FILE* fp;
    int clnt_sock;
}input_send;

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
	char payload[LOAD_SIZE];
}package;//fix the size:71


int base = 0;
int nextseqnum = 0;
int clnt_sock;

//其实数据封装应该就是个抽象概念，payload最长不超过64b，发送缓存为512b set the payload:57bytes
//着实比较麻烦，思路是先尝试怎么分包传送，然后实现窗口协议，再go_back_N,最后实现接受端
package* data_package(tcp_seq btcp_seq,tcp_seq btcp_ack,uint8_t data_off,uint8_t win_size,uint8_t flag,char* payload);

void tcp_send(package* tcp_package);

void Go_Back_N(int flag);

void* rdt_send(void* args);

void timeout(int clnt_sock);

void* rdt_rcv(void* args);

void start_timer();

void stop_timer();

void signal_handler(int m);

#endif