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
		// ftp������̣�����������Ӻ���������
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		handle_child(sess);
	}
	else
	{
		// nobody���̣�Э��ftp���������ftp�ͻ��˽������ӣ�why��
		//��Ϊ��һ��study�û���¼�ɹ��󣬻Ὣftp������̵��û�����Ϊstudy���û�����uid��gid��Ϊstudy�û���uid��gid
		//������ftp������̵�Ȩ�ޣ�eg��PORTģʽ�����Ƿ����������ӿͻ��ˣ���������Ҫbindһ����Ԫ�Ķ˿ںţ���Ԫ�˿�
		//��������ͨ�û���bind����ftp�������û��Ȩ����bind�˿ڣ���ʱ����Ҫnobody��Э��bind��Ԫ�˿���ftp�ͻ��˽�������
		//nobody��ftp������̵�Ȩ�����ĸ�
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		handle_parent(sess);
	}
}