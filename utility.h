#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

list<int>clients_list;
//保存客户端socket的列表

#define SERVER_IP "127.0.0.1"
//服务器端IP地址

#define SERVER_PORT 8888
//服务端端口号

#define EPOLL_SIZE 5000
//epoll 事件表的最大事件数

#define BUF_SIZE 0xFFF
//消息存储区的最大存储空间大小

#define SERVER_WELCOME "Welcome you join to the chat room! Your chat ID is:Client #%d"

#define SERVER_MESSAGE "ClientID %d say >> %s"

//退出
#define EXIT "EXIT"

#define CAUTION "There is only one int the char room!"

//fcntl()针对(文件)描述符提供控制,参数fd是被参数cmd操作(如下面的描述符)
int setnonblocking(int sockfd)
{
    fcntl(sockfd,F_SETFL,fcntl(sockfd,F_GETFD,0)|O_NONBLOCK);
    //F_SETFL->设置fd的状态标志,但部分标志是不能被修改的(比如访问模式标志!)
    //F_GETFD0>返回fd的相应文件标志
    //总体意思就是:返回sockfd本来的文件描述符标志再|上O_NONBLOCK 最后用新的文件描述符标志来设置更新!
    return 0;
}
//设置socket为非阻塞模式

void addFd(int epollfd,int fd,bool enable_et)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(enable_et)
    {
        ev.events = EPOLLIN|EPOLLET;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
    setnonblocking(fd);
    printf("fd added to epoll!\n");
}

int sendBroadcastmessage(int clientfd)
{
    //buf[BUF_SIZE]接受新的聊天信息
    //message[BUF_SIZE]保存格式信息
    char buf[BUF_SIZE],message[BUF_SIZE];
    bzero(buf,BUF_SIZE);
    bzero(message,BUF_SIZE);

    //接受信息
    printf("read from client(clientID = %d)\n",clientfd);
    int len = recv(clientfd,buf,BUF_SIZE,0);

    if(len <= 0)//意味着客户端断开连接
    {
        close(clientfd);
        clients_list.remove(clientfd); //服务器移除这个客户端
        printf("ClientID = %d closed.\n now there are %d client in the chat room\n",clientfd,(int)clients_list.size());
    }
    else   //传播信息
    {
        if(clients_list.size()==1)//这意味着仅有一个在chat room
        {
          send(clientfd,CAUTION,strlen(CAUTION),0);
          return len;
        }
        //format message to broadcast
        sprintf(message,SERVER_MESSAGE,clientfd,buf);

        list<int>::iterator it;
        for(it = clients_list.begin();it!=clients_list.end();it++)
        {
            if(*it!=clientfd)
            {
                if(send(*it,message,BUF_SIZE,0)<0)
                {
                    perror("error");
                    exit(-1);
                }
            }
        }
    }
    return len;
}
#endif 


