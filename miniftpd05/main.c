#include "common.h"
#include "sysutil.h"
#include "session.h"
#include "str.h"

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
	char *str1 = "		a b";
	char *str2 = "			  ";

	if (str_all_space(str1))
		printf("str1 all space\n");
	else
		printf("str1 not all space\n");

	if (str_all_space(str2))
		printf("str2 all space\n");
	else
		printf("str2 not all space\n");

	//char *str3 = "abcDef";		// ָ��ָ��һ���ַ����������������ܱ��޸ģ�����str_upper�����
	char str3[] = "abcDef";		//������һ���ַ����飬�����������ַ���������У�str3�������Ǳ�����ֻ�������������ݰᵽ�����ж���
	str_upper(str3);
	printf("str3=%s\n", str3);

	long long result = str_to_longlong("12345678901234");//һ���޷�������Ϊ4bytes������ܱ�ʾ�����ķ�Χ��40���ڣ���߳�����
														//һ���з�������Ϊ4bytes������ܱ�ʾ�����ķ�Χ��20����
	printf("result = %lld\n", result);


	int n = str_octal_to_uint("0711");
	printf("n=%d\n", n);


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
		conn = accept_timeout(listenfd, NULL, 0);
		if (conn == -1)
			ERR_EXIT("accept_tinmeout");

		pid = fork();
		if (pid == -1)
			ERR_EXIT("fork");

		if (pid == 0)
		{
			close(listenfd);
			sess.ctrl_fd = conn;
			begin_session(&sess);//��ͻ��˵�ͨ�Ź��̳���Ϊһ���Ự
		}
		else
			close(conn);
	}
	return 0;
}
