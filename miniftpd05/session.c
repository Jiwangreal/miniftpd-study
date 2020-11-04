#include "common.h"
#include "session.h"
#include "ftpproto.h"
#include "privparent.h"

void begin_session(session_t *sess)
{
	struct passwd *pw = getpwnam("nobody");
	if (pw == NULL)
		return;

	if (setegid(pw->pw_gid) < 0)
		ERR_EXIT("setegid");
	if (seteuid(pw->pw_uid) < 0)
		ERR_EXIT("seteuid");

	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");


	pid_t pid;
	pid = fork();
	if (pid < 0)
		ERR_EXIT("fork");

	if (pid == 0)
	{
		// ftp服务进程：处理控制连接和数据连接
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		handle_child(sess);
	}
	else
	{
		// nobody进程：协助ftp服务进程与ftp客户端进行连接，why？
		//因为当一个study用户登录成功后，会将ftp服务进程的用户名改为study的用户：将uid和gid改为study用户的uid和gid
		//来降低ftp服务进程的权限，eg：PORT模式，它是服务器端连接客户端，服务器端要bind一个二元的端口号，二元端口
		//不能用普通用户来bind，即ftp服务进程没有权限来bind端口，此时就需要nobody来协助bind二元端口与ftp客户端建立连接
		//nobody比ftp服务进程的权限来的高
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		handle_parent(sess);
	}
}