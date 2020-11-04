#include "ftpproto.h"
#include "sysutil.h"

void handle_child(session_t *sess)
{
	writen(sess->ctrl_fd, "220 (miniftpd 0.1)\r\n", strlen("220 (miniftpd 0.1)\r\n"));
	while (1)
	{
		memset(sess->cmdline, 0, sizeof(sess->cmdline));
		memset(sess->cmd, 0, sizeof(sess->cmd));
		memset(sess->arg, 0, sizeof(sess->arg));
		readline(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);//一行的最大值是：MAX_COMMAND_LINE

		// 得到一行数据，开始解析FTP命令与参数
		// 依据解析的参数，处理FTP命令
		//后期再将ftp服务进程改为登录用户进程
	}
}