#include "client_realiable.h"
using namespace std;

int main(){
    //先输入文件名，看文件是否能创建成功
    char filename[100] = "testdata3.txt";  //文件名
    // printf("Input filename to save: ");
    // cin.getline(filename,100);
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
    serv_addr.sin_port = htons(1240);  //端口
    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    expectedseqnum = 0;
    // tcp_seq ACK = 0;
    // package* sndpkt = data_package(0,ACK,0,WIN_SIZE,0,NULL);

    //读取服务器传回的数据
    char buffer[sizeof(package)];
	int nCount;
    package* recvPackage;
    while((nCount = read(sock,buffer,sizeof(package)))>0)
    {
    		recvPackage = (package*)buffer;
    		cout<<"expectedseqnum:"<<expectedseqnum<<endl;
    		if(recvPackage->header.btcp_seq == expectedseqnum)
    		{
    			fwrite(recvPackage->payload,recvPackage->header.data_off,1,fp);
    			package* sndpkt = data_package(expectedseqnum,recvPackage->header.btcp_seq,0,WIN_SIZE,0,NULL);
    			write(sock,(void*)sndpkt,sizeof(package));
    			cout<<expectedseqnum<<endl;
    			expectedseqnum = (expectedseqnum+1)%SEQ_MAX;  
    		}
    }
    // printf("Message form server: \n");
    // while( (nCount = read(sock, buffer, sizeof(package))) > 0 ){
    //     recvPackage = (package*)buffer;
    //     printf("%d %d %d %d\n",recvPackage->header.btcp_seq,recvPackage->header.btcp_ack,recvPackage->header.data_off,recvPackage->header.win_size);
    //     fwrite(recvPackage->payload, recvPackage->header.data_off, 1, fp);
    // }
    puts("File transfer success!");




    fclose(fp);
    //关闭套接字
    close(sock);
    return 0;
}


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

