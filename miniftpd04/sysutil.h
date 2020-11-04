#ifndef _SYS_UTIL_H_
#define _SYS_UTIL_H_

#include "common.h"

//下面的函数全是系统工具

//c/s架构：对于服务器端来说：创建fd，bind，listen，accept连接，处理连接
int tcp_server(const char *host, unsigned short port);

//获取本地ip地址的函数
int getlocalip(char *ip);

//将一个fd设置为非阻塞模式
void activate_nonblock(int fd);

//将一个fd去掉非阻塞模式，即为阻塞模式
void deactivate_nonblock(int fd);

//读超时
int read_timeout(int fd, unsigned int wait_seconds);

//写超时
int write_timeout(int fd, unsigned int wait_seconds);

//接收超时
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

//连接超时
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count);
ssize_t recv_peek(int sockfd, void *buf, size_t len);
//按行读取函数
ssize_t readline(int sockfd, void *buf, size_t maxline);

//发送fd
void send_fd(int sock_fd, int fd);
//接收fd
int recv_fd(const int sock_fd);

#endif /* _SYS_UTIL_H_ */

