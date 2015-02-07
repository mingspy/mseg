#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <exception>
#include <iostream>
#include <signal.h>

using namespace std;

#define MAXEPOLLSIZE 10000
#define BUFFER_SIZE 400
#define MAX_EVENTS 10
int port = 59834;
int setnonblocking(int sockfd)
{
    if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1) {
         return -1;
    }
    return 0;
}

int server()  
{
    int server_sockfd;// 服务器端套接字   
    int client_sockfd;// 客户端套接字   
    int len;   
    struct sockaddr_in my_addr;   // 服务器网络地址结构体   
    struct sockaddr_in remote_addr; // 客户端网络地址结构体   
    char buf[BUFFER_SIZE];  // 数据传送的缓冲区   
    memset(&my_addr,0,sizeof(my_addr)); // 数据初始化--清零   
    my_addr.sin_family=AF_INET; // 设置为IP通信   
    my_addr.sin_addr.s_addr=INADDR_ANY;// 服务器IP地址--允许连接到所有本地地址上   
    my_addr.sin_port=htons(port); // 服务器端口号   

    /* 设置每个进程允许打开的最大文件数 */
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = MAXEPOLLSIZE;
    if (setrlimit(RLIMIT_NOFILE, &rt) == -1) 
    {
        perror("setrlimit error");
        return -1;
    }

    // 创建服务器端套接字--IPv4协议，面向连接通信，TCP协议
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)   
    {     
        perror("socket");   
        return 1;   
    }   

    int opt = 1;
    setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (setnonblocking(server_sockfd) < 0) {
        perror("setnonblock error");
    }

    // 将套接字绑定到服务器的网络地址上
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)   
    {   
        perror("bind");   
        return 1;   
    }   
    // 监听连接请求--监听队列长度为500 
    listen(server_sockfd,500);   
    socklen_t sin_size=sizeof(struct sockaddr_in); 

    // 创建一个epoll句柄
    int epoll_fd;
    epoll_fd=epoll_create(MAX_EVENTS);
    if(epoll_fd==-1)
    {
        perror("epoll_create failed");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;// epoll事件结构体
    struct epoll_event events[MAX_EVENTS];// 事件监听队列
    ev.events=EPOLLIN | EPOLLET;
    ev.data.fd=server_sockfd;

    // 向epoll注册server_sockfd监听事件
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,server_sockfd,&ev)==-1)
    {
        perror("epll_ctl:server_sockfd register failed");
        exit(EXIT_FAILURE);
    }
    int nfds;// epoll监听事件发生的个数
    // 循环接受客户端请求   
try{
    while(1)
    {
        // 等待事件发生
        nfds=epoll_wait(epoll_fd,events,MAX_EVENTS,-1);
        if(nfds==-1)
        {
            perror("start epoll_wait failed");
            //exit(EXIT_FAILURE);
        }
        int i;
        for(i=0;i<nfds;i++)
        {
            // 客户端有新的连接请求
            if(events[i].data.fd==server_sockfd)
            {
                // 等待客户端连接请求到达
                if((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0)
                {   
                    perror("accept client_sockfd failed");   
                    //exit(EXIT_FAILURE);
                }
                // 向epoll注册client_sockfd监听事件
                setnonblocking(client_sockfd);
                ev.events=EPOLLIN | EPOLLET;
                ev.data.fd=client_sockfd;
                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_sockfd,&ev)==-1)
                {
                    perror("epoll_ctl:client_sockfd register failed");
                    //exit(EXIT_FAILURE);
                }
                printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));
            }
            // 客户端有数据发送过来
            else
            {
                len=recv(events[i].data.fd,buf,BUFFER_SIZE,0);
                if(len<0)
                {
                    perror("receive from client failed");
                    //exit(EXIT_FAILURE);
                }
                printf("receive from client:%s\n",buf);
                send(events[i].data.fd,"I have received your message.",30,0);
            }
        }
    }
} catch(exception e){
    cerr <<"exception"<<e.what()<<endl;
}catch(...){
    cerr <<"socket exception"<<endl;
}

    return 0;   
}  
 

int client() 
{   
    int client_sockfd;   
    int len;   
    struct sockaddr_in remote_addr; // 服务器端网络地址结构体   
    char buf[BUFFER_SIZE];  // 数据传送的缓冲区   
    memset(&remote_addr,0,sizeof(remote_addr)); // 数据初始化--清零   
    remote_addr.sin_family=AF_INET; // 设置为IP通信   
    remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");// 服务器IP地址   
    remote_addr.sin_port=htons(port); // 服务器端口号   
    // 创建客户端套接字--IPv4协议，面向连接通信，TCP协议 
    if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)   
    {   
        perror("client socket creation failed");   
        exit(EXIT_FAILURE);
    }   
    // 将套接字绑定到服务器的网络地址上 
    if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)   
    {   
        perror("connect to server failed");   
        exit(EXIT_FAILURE);
    }  

    // 循环监听服务器请求   
    while(1)
    {
        printf("Please input the message:");
        scanf("%s",buf);
        // exit
        if(strcmp(buf,"exit")==0)
        {
            break;
        }
        send(client_sockfd,buf,BUFFER_SIZE,0);
        // 接收服务器端信息 
        len=recv(client_sockfd,buf,BUFFER_SIZE,0);
        printf("\nreceive from server:%s\n",buf);
        if(len<0)
        {
            perror("receive from server failed");
            exit(EXIT_FAILURE);
        }
    }
    close(client_sockfd);// 关闭套接字   
    return 0;
}

int main(int argc, char ** argv){
    signal(SIGPIPE,SIG_IGN);
    if (argc == 2){
        client();
    }
    else{
        server();
    }
    return 0;
}
