#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"

void handle_child(session_t *sess)
{
	//ÿ��ָ��������\r\n������ftpЭ�����涨��
	writen(sess->ctrl_fd, "220 (miniftpd 0.1)\r\n", strlen("220 (miniftpd 0.1)\r\n"));
	int ret;
	while (1)
	{
		memset(sess->cmdline, 0, sizeof(sess->cmdline));
		memset(sess->cmd, 0, sizeof(sess->cmd));
		memset(sess->arg, 0, sizeof(sess->arg));
		ret = readline(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);
		if (ret == -1)//�ر�ftp������̣����⣬��Ҫ�ر�nobody���̣�ͨ��socketpair�ķ�ʽ�������ټ���
			ERR_EXIT("readline");
		else if (ret == 0)//��ʾ�ͻ��˶Ͽ�����
			exit(EXIT_SUCCESS);
		
		//�ͻ������ӹ����Ļ����ᷢ��USER jjl\r\n�ַ������������˽����յ��ַ�����ӡ����
		//��USER jjl\r\n�ַ������ÿո�ָ���2��
		printf("cmdline=[%s]\n", sess->cmdline);
		// ȥ��\r\n
		str_trim_crlf(sess->cmdline);
		printf("cmdline=[%s]\n", sess->cmdline);
		// ����FTP���������
		str_split(sess->cmdline, sess->cmd, sess->arg, ' ');
		printf("cmd=[%s] arg=[%s]\n", sess->cmd, sess->arg);
		// ������ת��Ϊ��д
		str_upper(sess->cmd);
		// ����FTP����
	}
}