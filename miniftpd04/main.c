#include "common.h"
#include "sysutil.h"
#include "session.h"

/*
typedef struct session
{
	// ��������
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// ���ӽ���ͨ��
	int parent_fd;
	int child_fd;
} session_t;
*/


int main(void)
{
	//�ж��Ƿ���root�û�����
	if (getuid() != 0)
	{
		fprintf(stderr, "miniftpd: must be started as root\n");
		exit(EXIT_FAILURE);
	}


	session_t sess = 
	{
		/* �������� */
		-1, "", "", "",
		/* ���ӽ���ͨ�� */
		-1, -1
	};
	
	int listenfd = tcp_server(NULL, 5188);
	int conn;
	pid_t pid;

	while (1)
	{
		conn = accept_timeout(listenfd, NULL, 0);//NULL���Եȷŵĵ�ַ��0����ʹ�ó�ʱ
		if (conn == -1)
			ERR_EXIT("accept_tinmeout");

		pid = fork();
		if (pid == -1)
			ERR_EXIT("fork");

		if (pid == 0)//����һ���Ự
		{
			close(listenfd);//�ӽ���
			sess.ctrl_fd = conn;
			begin_session(&sess);
		}
		else
			close(conn);//������
	}
	return 0;
}
