#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
using namespace std;

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

// struct my_struct my_struct_client;
// write(fd,(void *)&my_struct_client,sizeof(struct my_struct);


// char buffer[sizeof(struct my_struct)];
// struct *my_struct_server;
// read(fd,(void *)buffer,sizeof(struct my_struct));
// my_struct_server=(struct my_struct *)buffer;   


int main(){
    //先输入文件名，看文件是否能创建成功
    char filename[100] = {0};  //文件名
    printf("Input filename to save: ");
    cin.getline(filename,100);
    FILE *fp = fopen(filename, "wb");  //以二进制方式打开（创建）文件
    if(fp == NULL){
        printf("Cannot open file, press any key to exit!\n");
        system("pause");
        exit(0);
    }
    //创建套接字
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    //向服务器（特定的IP和端口）发起请求
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(1234);  //端口
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    
    //char bufSend[BUF_SIZE] = {0};
    //char bufRecv[BUF_SIZE] = {0};

    //读取服务器传回的数据
    char buffer[sizeof(package)];


    int nCount;
    package* recvPackage;
    printf("Message form server: \n");
    while( (nCount = read(sock, buffer, sizeof(package))) > 0 ){
        recvPackage = (package*)buffer;
        printf("%d %d %d %d\n",recvPackage->header.btcp_seq,recvPackage->header.btcp_ack,recvPackage->header.data_off,recvPackage->header.win_size);
        fwrite(recvPackage->payload, recvPackage->header.data_off, 1, fp);
    }
    puts("File transfer success!");
    fclose(fp);

    // read(sock, buffer, sizeof(package));
    // recvPackage = (package*)buffer;
    // printf("Message form server: \n");
    // printf("%d %d %d %d\n",recvPackage->header.btcp_seq,recvPackage->header.btcp_ack,recvPackage->header.data_off,recvPackage->header.win_size);
    // for (int i = 0; i < sizeof(recvPackage->payload); ++i)
    // {
    //     printf("%c",recvPackage->payload[i]);
    //     /* code */
    // }
    //关闭套接字
    close(sock);
    return 0;
}