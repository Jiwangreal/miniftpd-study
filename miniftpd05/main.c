#include "common.h"
#include "sysutil.h"
#include "session.h"
#include "str.h"

/*
typedef struct session
{
	// 控制连接
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// 父子进程通道
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

	//char *str3 = "abcDef";		// 指针指向一个字符串常量，常量不能被修改，调用str_upper会出错
	char str3[] = "abcDef";		//定义了一个字符数组，将常量存入字符数组变量中，str3本质上是变量，只不过将常量内容搬到变量中而已
	str_upper(str3);
	printf("str3=%s\n", str3);

	long long result = str_to_longlong("12345678901234");//一个无符号整型为4bytes，最大能表示的数的范围是40几亿，这边超过了
														//一个有符号整型为4bytes，最大能表示的数的范围是20几亿
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
		/* 控制连接 */
		-1, "", "", "",
		/* 父子进程通道 */
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
			begin_session(&sess);//与客户端的通信过程抽象为一个会话
		}
		else
			close(conn);
	}
	return 0;
}
