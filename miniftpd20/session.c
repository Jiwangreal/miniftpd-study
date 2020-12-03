#include "common.h"
#include "session.h"
#include "ftpproto.h"
#include "privparent.h"
#include "privsock.h"
#include "sysutil.h"

void begin_session(session_t *sess)
{
	activate_oobinline(sess->ctrl_fd);//����fd����oobinline��ͨ������ģʽ����������
	/*
	���⣬�����ܹ���׽��һ���ź�SIGURG���Ա������ս�������
	���н������ݵ�����ʱ�򣬻�������źŷ��͸���ǰ���̣�Ȼ��ǰ���̲��ܹ����ս�������
	SIGURG�Ŀ���Ҫ����fcntl�Լ������е�F_SETOWN���������ĳ��fd��������SIGURG��SIGIO�źŸ��������
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
		// ftp�������
		/*
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		*/
		priv_sock_set_child_context(sess);
		handle_child(sess);
	}
	else
	{

		// nobody����
		
		/*
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		*/
		priv_sock_set_parent_context(sess);
		handle_parent(sess);
	}
}