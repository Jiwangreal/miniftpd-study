#include "common.h"
#include "session.h"
#include "ftpproto.h"
#include "privparent.h"

//begin_session：包含一个nobody进程，一个服务进程，这俩作为一个会话
void begin_session(session_t *sess)
{
	//man getpwnam，对应的是/etc/passwd中的密码信息
	struct passwd *pw = getpwnam("nobody");
	if (pw == NULL)
		return;
	//将父进程改成nobody进程
	//要先改组id，再改用户id，因为先改用户id，可能没有权限再改组id了
	//没改之前，因为是root用户启动，所以uid和gid都为0
	if (setegid(pw->pw_gid) < 0)//将当前进程的有效组id改为pw_gid
		ERR_EXIT("setegid");
	if (seteuid(pw->pw_uid) < 0)//将当前进程的有效用户id改为pw_uid
		ERR_EXIT("seteuid");


	//父子进程通信使用socketpair，一对套接字
	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");


	pid_t pid;
	pid = fork();
	if (pid < 0)
		ERR_EXIT("fork");

	if (pid == 0)
	{
		// 子进程为：ftp服务进程
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		handle_child(sess);
	}
	else
	{
		// 父进程为nobody进程
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		handle_parent(sess);
	}
}