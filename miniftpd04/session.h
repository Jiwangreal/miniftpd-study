#ifndef _SESSION_H_
#define _SESSION_H_

#include "common.h"

typedef struct session
{
	// 控制连接
	int ctrl_fd;//已连接socket
	char cmdline[MAX_COMMAND_LINE];//命令行
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];//命令行参数

	// 父子进程通道
	int parent_fd;//父进程的fd
	int child_fd;//子进程的fd
} session_t;
void begin_session(session_t *sess);

#endif /* _SESSION_H_ */
