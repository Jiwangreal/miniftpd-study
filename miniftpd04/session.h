#ifndef _SESSION_H_
#define _SESSION_H_

#include "common.h"

typedef struct session
{
	// ��������
	int ctrl_fd;//������socket
	char cmdline[MAX_COMMAND_LINE];//������
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];//�����в���

	// ���ӽ���ͨ��
	int parent_fd;//�����̵�fd
	int child_fd;//�ӽ��̵�fd
} session_t;
void begin_session(session_t *sess);

#endif /* _SESSION_H_ */
