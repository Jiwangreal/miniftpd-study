#include "privsock.h"
#include "common.h"
#include "sysutil.h"

void priv_sock_init(session_t *sess)
{
	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");

	//socket对初始化
	sess->parent_fd = sockfds[0];
	sess->child_fd = sockfds[1];
}

void priv_sock_close(session_t *sess)
{
	if (sess->parent_fd != -1)
	{
		close(sess->parent_fd);
		sess->parent_fd = -1;
	}

	if (sess->child_fd != -1)
	{
		close(sess->child_fd);
		sess->child_fd = -1;
	}
}

void priv_sock_set_parent_context(session_t *sess)
{
	if (sess->child_fd != -1)//父进程不需要父进程的socket
	{
		close(sess->child_fd);
		sess->child_fd = -1;
	}
}

void priv_sock_set_child_context(session_t *sess)
{
	if (sess->parent_fd != -1)//子进程不需要父进程的socket
	{
		close(sess->parent_fd);
		sess->parent_fd = -1;
	}
}

//ftp服务进程向nobody进程发送的消息，发送的消息cmd是1个字节
void priv_sock_send_cmd(int fd, char cmd)
{
	int ret;
	ret = writen(fd, &cmd, sizeof(cmd));
	if (ret != sizeof(cmd))
	{
		fprintf(stderr, "priv_sock_send_cmd error\n");
		exit(EXIT_FAILURE);
	}
}

//nobody进程接收ftp服务进程的消息，接收的消息cmd是1个字节
char priv_sock_get_cmd(int fd)
{
	char res;
	int ret;
	ret = readn(fd, &res, sizeof(res));
	if (ret != sizeof(res))
	{
		fprintf(stderr, "priv_sock_get_cmd error\n");
		exit(EXIT_FAILURE);
	}

	return res;
}

//nobody服务进程向ftp进程发送的消息，发送的消息cmd是1个字节
void priv_sock_send_result(int fd, char res)
{
	int ret;
	ret = writen(fd, &res, sizeof(res));
	if (ret != sizeof(res))
	{
		fprintf(stderr, "priv_sock_send_result error\n");
		exit(EXIT_FAILURE);
	}
}

//ftp进程接收nobody服务进程的消息，接收的消息cmd是1个字节
char priv_sock_get_result(int fd)
{
	char res;
	int ret;
	ret = readn(fd, &res, sizeof(res));
	if (ret != sizeof(res))
	{
		fprintf(stderr, "priv_sock_get_result error\n");
		exit(EXIT_FAILURE);
	}

	return res;
}

//发送一个整数，即发送端口
void priv_sock_send_int(int fd, int the_int)
{
	int ret;
	ret = writen(fd, &the_int, sizeof(the_int));
	if (ret != sizeof(the_int))
	{
		fprintf(stderr, "priv_sock_send_int error\n");
		exit(EXIT_FAILURE);
	}
}
//接收一个整数，接收一个端口
int priv_sock_get_int(int fd)
{
	int the_int;
	int ret;
	ret = readn(fd, &the_int, sizeof(the_int));
	if (ret != sizeof(the_int))
	{
		fprintf(stderr, "priv_sock_get_int error\n");
		exit(EXIT_FAILURE);
	}

	return the_int;
}
//发送一个字符串
void priv_sock_send_buf(int fd, const char *buf, unsigned int len)
{
	priv_sock_send_int(fd, (int)len);//对于一个不定长的字符串来说呢，先将长度发送过去
	int ret = writen(fd, buf, len);//再发送实际的字符串
	if (ret != (int)len)
	{
		fprintf(stderr, "priv_sock_send_buf error\n");
		exit(EXIT_FAILURE);
	}
}

//接收一个字符串
void priv_sock_recv_buf(int fd, char *buf, unsigned int len)
{
	unsigned int recv_len = (unsigned int)priv_sock_get_int(fd);//接收的时候，先接收一个整数，看看字符串有多少个字节
	if (recv_len > len)
	{
		fprintf(stderr, "priv_sock_recv_buf error\n");
		exit(EXIT_FAILURE);
	}

	int ret = readn(fd, buf, recv_len);
	if (ret != (int)recv_len)
	{
		fprintf(stderr, "priv_sock_recv_buf error\n");
		exit(EXIT_FAILURE);
	}
}

//发送fd
void priv_sock_send_fd(int sock_fd, int fd)
{
	send_fd(sock_fd, fd);
}

//接收fd
int priv_sock_recv_fd(int sock_fd)
{
	return recv_fd(sock_fd);
}


