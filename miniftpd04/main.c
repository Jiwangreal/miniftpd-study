#include "common.h"
#include "sysutil.h"
#include "session.h"

/*
typedef struct session
{
	// 控制连接
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// 父子进程通道
	int parent_fd;
	int child_fd;
} session_t;
*/


int main(void)
{
	//判断是否以root用户启动
	if (getuid() != 0)
	{
		fprintf(stderr, "miniftpd: must be started as root\n");
		exit(EXIT_FAILURE);
	}


	session_t sess = 
	{
		/* 控制连接 */
		-1, "", "", "",
		/* 父子进程通道 */
		-1, -1
	};
	
	int listenfd = tcp_server(NULL, 5188);
	int conn;
	pid_t pid;

	while (1)
	{
		conn = accept_timeout(listenfd, NULL, 0);//NULL：对等放的地址，0：不使用超时
		if (conn == -1)
			ERR_EXIT("accept_tinmeout");

		pid = fork();
		if (pid == -1)
			ERR_EXIT("fork");

		if (pid == 0)//开启一个会话
		{
			close(listenfd);//子进程
			sess.ctrl_fd = conn;
			begin_session(&sess);
		}
		else
			close(conn);//父进程
	}
	return 0;
}
