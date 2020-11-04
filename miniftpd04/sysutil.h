#ifndef _SYS_UTIL_H_
#define _SYS_UTIL_H_

#include "common.h"

//����ĺ���ȫ��ϵͳ����

//c/s�ܹ������ڷ���������˵������fd��bind��listen��accept���ӣ���������
int tcp_server(const char *host, unsigned short port);

//��ȡ����ip��ַ�ĺ���
int getlocalip(char *ip);

//��һ��fd����Ϊ������ģʽ
void activate_nonblock(int fd);

//��һ��fdȥ��������ģʽ����Ϊ����ģʽ
void deactivate_nonblock(int fd);

//����ʱ
int read_timeout(int fd, unsigned int wait_seconds);

//д��ʱ
int write_timeout(int fd, unsigned int wait_seconds);

//���ճ�ʱ
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

//���ӳ�ʱ
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count);
ssize_t recv_peek(int sockfd, void *buf, size_t len);
//���ж�ȡ����
ssize_t readline(int sockfd, void *buf, size_t maxline);

//����fd
void send_fd(int sock_fd, int fd);
//����fd
int recv_fd(const int sock_fd);

#endif /* _SYS_UTIL_H_ */

