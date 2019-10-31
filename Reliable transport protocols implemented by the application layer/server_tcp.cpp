//此版本已经实现分包传输

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define BUF_SIZE 512
#define LOAD_SIZE 57
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

//其实数据封装应该就是个抽象概念，payload最长不超过64b，发送缓存为512b set the payload:57bytes
//着实比较麻烦，思路是先尝试怎么分包传送，然后实现窗口协议，再go_back_N,最后实现接受端
package* data_package(tcp_seq btcp_seq,tcp_seq btcp_ack,uint8_t data_off,uint8_t win_size,uint8_t flag,char* payload);

void tcp_send(package* tcp_package);

void Go_Back_N(int flag);

package* data_package(tcp_seq btcp_seq,tcp_seq btcp_ack,uint8_t data_off,uint8_t win_size,uint8_t flag,char* payload)
{
	package* newPackage = (package*)malloc(sizeof(package));
	newPackage->header.btcp_sport = 0;
	newPackage->header.btcp_dport = 0;
	newPackage->header.btcp_seq = btcp_seq;
	newPackage->header.btcp_ack = btcp_ack;
	newPackage->header.data_off = data_off;
	newPackage->header.win_size = win_size;
	newPackage->header.flag = flag;
	for (uint8_t i = 0; i < data_off; ++i)
	{
		newPackage->payload[i] = payload[i];
	}
	return newPackage;
}


// struct my_struct my_struct_client;
// write(fd,(void *)&my_struct_client,sizeof(struct my_struct);


// char buffer[sizeof(struct my_struct)];
// struct *my_struct_server;
// read(fd,(void *)buffer,sizeof(struct my_struct));
// my_struct_server=(struct my_struct *)buffer;    

int main()
{
	char *filename = "testdata.txt";  //文件名
    FILE *fp = fopen(filename, "rb");  //以二进制方式打开文件
    if(fp == NULL){
        printf("Cannot open file, press any key to exit!\n");
        system("pause");
        exit(0);
    }

    //创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //参数 AF_INET 表示使用 IPv4 地址，SOCK_STREAM 表示使用面向连接的套接字，IPPROTO_TCP 表示使用 TCP 协议
    //将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(1234);  //端口
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);
    //接收客户端请求
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

    char buffer[BUF_SIZE] = {0};

    int nCount = 0;
    int seq = 0;
    int ack = 0;
    while((nCount = fread(buffer,1,LOAD_SIZE,fp))>0)
    {
    	package* send_package = data_package(seq,0,nCount,WIN_SIZE,0,buffer);
    	seq = seq + 64;//固定每个package64bytes
        write(clnt_sock,(void*)send_package,sizeof(package));
    }
    shutdown(clnt_sock,SHUT_WR);
    // fread(buffer,1,LOAD_SIZE,fp);
    // package* send_package = data_package(0,0,LOAD_SIZE,WIN_SIZE,0,buffer);
    // write(clnt_sock,(void*)send_package,sizeof(package));

    fclose(fp);
    close(clnt_sock);
    close(serv_sock);
    return 0;


}

