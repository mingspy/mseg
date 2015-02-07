#include <event.h>
#include <iostream>
#include "../thirdparty/libevent-2.0.22-stable/event-internal.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <signal.h>

#define BUFFER_SIZE 1024
using namespace std;

event ev;
event evclient;
timeval tv;

void timer_cb(int fd, short event, void * arg){
    cout<<"timer is raised"<<endl;
    event_add(&ev,&tv);
}

void test_timer(){
    event_base * base = event_init();        
    cout<<base->evsel->name<<endl; tv.tv_sec = 10; tv.tv_usec = 0; evtimer_set(&ev,timer_cb, NULL);
    event_add(&ev,&tv);
    event_base_dispatch(base);
}

void client_can_read(int fd, short type, void * arg){
try{
    char buf[BUFFER_SIZE];
    int len;
    if (len = recv(fd, buf, BUFFER_SIZE,0) <= 0){
        perror("read error");
        event_del(&evclient);
        return;
    }
    cout<<"received:"<<buf<<endl;
    send(fd, "success",8,0);
    event_add(&evclient,NULL);
}
catch(...){
    perror("read client");
}
}

void server_can_read(int fd, short type, void * arg){

try{
    sockaddr_in remote_addr;
    socklen_t sin_size=sizeof(struct sockaddr_in); 
    int client_sockfd;
    if((client_sockfd=accept(fd,(struct sockaddr *)&remote_addr,&sin_size))<0) {   
        perror("accept client_sockfd failed");   
        exit(EXIT_FAILURE);
    }
    cout<<"accept client "<<inet_ntoa(remote_addr.sin_addr)<<endl;
    event_set(&evclient, client_sockfd, EV_READ|EV_PERSIST,client_can_read,NULL);
    event_add(&evclient,NULL);
}catch(...){
    perror("server");
}
}

int main(void){
    signal(SIGPIPE,SIG_IGN);
//    test_timer();
    int srv_socketfd = socket(PF_INET, SOCK_STREAM,0);
    if (srv_socketfd < 0){
        perror("create socket");
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(59834);

    if (bind(srv_socketfd, (sockaddr *)&addr, sizeof(struct sockaddr)) < 0){
        perror("binding");
        return -1;
    }

    listen(srv_socketfd, 50);
    event_base * base = event_init();        
    event_set(&ev,srv_socketfd,EV_READ|EV_PERSIST,server_can_read,NULL);
    //event_base_set(base,&ev);
    event_add(&ev,NULL);
try{
    event_base_dispatch(base);
}catch(...){
    perror("loop");
    cout<<"end loop"<<endl;
}
    cout<<"end"<<endl;
}
