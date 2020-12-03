#include "common.h"
#include "session.h"
#include "ftpproto.h"
#include "privparent.h"
#include "privsock.h"
#include "sysutil.h"

void begin_session(session_t *sess)
{
	activate_oobinline(sess->ctrl_fd);//控制fd开启oobinline，通过带外模式来接收数据
	/*
	此外，进程能够捕捉到一个信号SIGURG，以便它接收紧急数据
	当有紧急数据到来的时候，会产生该信号发送给当前进程，然后当前进程才能够接收紧急数据
	SIGURG的开启要调用fcntl以及及其中的F_SETOWN：她会接受某个fd所产生的SIGURG或SIGIO信号给这个进程
	*/

	/*
	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");
	*/

	priv_sock_init(sess);

	pid_t pid;
	pid = fork();
	if (pid < 0)
		ERR_EXIT("fork");

	if (pid == 0)
	{
		// ftp服务进程
		/*
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		*/
		priv_sock_set_child_context(sess);
		handle_child(sess);
	}
	else
	{

		// nobody进程
		
		/*
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		*/
		priv_sock_set_parent_context(sess);
		handle_parent(sess);
	}
}