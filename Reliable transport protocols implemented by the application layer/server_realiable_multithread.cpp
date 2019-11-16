//此版本已经实现分包传输
#include "server_realiable_multithread.h"

using namespace std;
int finish = 0;
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
    serv_addr.sin_port = htons(1230);  //端口
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);
    //接收客户端请求
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);

    //定时器信号设置
    signal(SIGALRM, signal_handler);


    pthread_t threads[2];
    int s = 0;
    int rc;
    while(1)
    {
        if(s == 0)
        {
            input_send in = {.fp = fp,.clnt_sock = clnt_sock};
            rc = pthread_create(&threads[s], NULL, rdt_send, (void *)&in);
            if (rc){
                cout << "Error:无法创建线程," << rc << endl;
                exit(-1);
            }
            s++;
        }
        else if(s == 1)
        {
            rc = pthread_create(&threads[s], NULL, rdt_rcv, (void *)&clnt_sock);
            if (rc){
                cout << "Error:无法创建线程," << rc << endl;
                exit(-1);
            }
            s++;
        }
        // if(result_send == FULL)
        // {
        //     //cout<<"FULL"<<endl;
        // }
        // else 
        if(result_send == EMPTY)
        {

            finish = 1;
            break;
            // if(result_recv == nextseqnum)
            // {
            //     cout<<"Finish"<<endl;
            //     break;
            // }
        }
    }
    //设置发送缓存
    // int buffer_send = 8;
    // package* package_buffer_send[buffer_send];
    shutdown(clnt_sock,SHUT_WR);
    fclose(fp);
    close(clnt_sock);
    close(serv_sock);
    return 0;


}

package* package_buffer_send[SEQ_MAX];


void* rdt_send(void* args)
{
    input_send in = *((input_send*)args);
    FILE* fp = in.fp;
    int clnt_sock = in.clnt_sock;
    while(1)
    {
        if(finish)
            break;
        char buffer[BUF_SIZE];
        int nCount;
        if((nextseqnum>=SEQ_MAX-WIN_SIZE&&nextseqnum<base + WIN_SIZE)||(base+WIN_SIZE<SEQ_MAX&&nextseqnum<base+WIN_SIZE)||(nextseqnum<(base+WIN_SIZE)%SEQ_MAX)){
            //cout<<"yes"<<endl;
            if((nCount = fread(buffer,1,LOAD_SIZE,fp))>0)
            {
                package_buffer_send[nextseqnum] = data_package(nextseqnum,0,nCount,WIN_SIZE,0,buffer);
                cout<<"send window:"<<base<<' '<<nextseqnum<<endl;
                write(clnt_sock,(void*)package_buffer_send[nextseqnum],sizeof(package));
            }
            else
            {
                //finish close
                start_timer();
                cout<<"EMPTY"<<endl;
                result_send = EMPTY;
            }
            if(base == nextseqnum)
            {
                start_timer();
                //start_timer
            }
            nextseqnum = (nextseqnum+1)%SEQ_MAX;
        }
        else
        {
            //cout<<"nextseqnum:"<<nextseqnum<<endl;
            result_send = FULL;//full of window
            //do nothing   
        }
        usleep(1000);
    }
    pthread_exit(NULL);
}

void timeout(int clnt_sock)
{
    //start_timer
    cout<<"timeout"<<endl;
    if(base >=nextseqnum)
    {
        stop_timer();
        package_buffer_send[nextseqnum-1]->header.flag = 1;
        package_buffer_send[nextseqnum-1]->header.btcp_seq = nextseqnum;
        package_buffer_send[nextseqnum-1]->header.data_off = 0;
        write(clnt_sock,(void*)package_buffer_send[nextseqnum-1],sizeof(package));
    }
    for (int i = base; i < nextseqnum; ++i)
    {
        cout<<"send again:"<<i<<endl;
        package_buffer_send[i]->header.flag = 1;
        write(clnt_sock,(void*)package_buffer_send[i],sizeof(package));
    }
}

void* rdt_rcv(void* args)
{
    int clnt_sock = *((int*)args);
    while(1)
    {
        if(finish)
            break;
        char buffer[sizeof(package)];


        int nCount;
        package* recvPackage;
        if((nCount = read(clnt_sock, buffer, sizeof(package)))>0)
        {
            printf("recv!\n");
            recvPackage = (package*)buffer;
            base = (recvPackage->header.btcp_ack+1)%SEQ_MAX;
            cout<<"base:"<<base<<endl;
            if(base == nextseqnum)
            {
                stop_timer();
                //stop_timer
            }
            else{
                start_timer();
                //start_timer
            }
            result_recv = recvPackage->header.btcp_seq;
        }
        usleep(1000);
    }
    pthread_exit(NULL);
}

void start_timer()
{
    cout<<"start"<<endl;
    struct itimerval itv;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = TIME_INTERVAL;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = TIME_INTERVAL;
    setitimer(ITIMER_REAL, &itv, &oldtv);
}

void stop_timer()
{
    cout<<"end"<<endl;
    struct itimerval itv;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = TIME_INTERVAL;
    itv.it_value.tv_sec = 0;  //不触发
    itv.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &itv, &oldtv);
}

void signal_handler(int m)
{
    timeout(clnt_sock);
}