#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"

void handle_child(session_t *sess)
{
	//每条指令后面加上\r\n，这是ftp协议所规定的
	writen(sess->ctrl_fd, "220 (miniftpd 0.1)\r\n", strlen("220 (miniftpd 0.1)\r\n"));
	int ret;
	while (1)
	{
		memset(sess->cmdline, 0, sizeof(sess->cmdline));
		memset(sess->cmd, 0, sizeof(sess->cmd));
		memset(sess->arg, 0, sizeof(sess->arg));
		ret = readline(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);
		if (ret == -1)//关闭ftp服务进程，此外，还要关闭nobody进程，通过socketpair的方式，后期再加上
			ERR_EXIT("readline");
		else if (ret == 0)//表示客户端断开连接
			exit(EXIT_SUCCESS);
		
		//客户端连接过来的话，会发送USER jjl\r\n字符串，这里服务端将接收的字符串打印出来
		//将USER jjl\r\n字符串，用空格分隔成2段
		printf("cmdline=[%s]\n", sess->cmdline);
		// 去除\r\n
		str_trim_crlf(sess->cmdline);
		printf("cmdline=[%s]\n", sess->cmdline);
		// 解析FTP命令与参数
		str_split(sess->cmdline, sess->cmd, sess->arg, ' ');
		printf("cmd=[%s] arg=[%s]\n", sess->cmd, sess->arg);
		// 将命令转换为大写
		str_upper(sess->cmd);
		// 处理FTP命令
	}
}