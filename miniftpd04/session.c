#include "common.h"
#include "session.h"
#include "ftpproto.h"
#include "privparent.h"

//begin_session������һ��nobody���̣�һ��������̣�������Ϊһ���Ự
void begin_session(session_t *sess)
{
	//man getpwnam����Ӧ����/etc/passwd�е�������Ϣ
	struct passwd *pw = getpwnam("nobody");
	if (pw == NULL)
		return;
	//�������̸ĳ�nobody����
	//Ҫ�ȸ���id���ٸ��û�id����Ϊ�ȸ��û�id������û��Ȩ���ٸ���id��
	//û��֮ǰ����Ϊ��root�û�����������uid��gid��Ϊ0
	if (setegid(pw->pw_gid) < 0)//����ǰ���̵���Ч��id��Ϊpw_gid
		ERR_EXIT("setegid");
	if (seteuid(pw->pw_uid) < 0)//����ǰ���̵���Ч�û�id��Ϊpw_uid
		ERR_EXIT("seteuid");


	//���ӽ���ͨ��ʹ��socketpair��һ���׽���
	int sockfds[2];
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sockfds) < 0)
		ERR_EXIT("socketpair");


	pid_t pid;
	pid = fork();
	if (pid < 0)
		ERR_EXIT("fork");

	if (pid == 0)
	{
		// �ӽ���Ϊ��ftp�������
		close(sockfds[0]);
		sess->child_fd = sockfds[1];
		handle_child(sess);
	}
	else
	{
		// ������Ϊnobody����
		close(sockfds[1]);
		sess->parent_fd = sockfds[0];
		handle_parent(sess);
	}
}