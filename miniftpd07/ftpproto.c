#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"

void ftp_reply(session_t *sess, int status, const char *text);
static void do_user(session_t *sess);
static void do_pass(session_t *sess);

void handle_child(session_t *sess)
{
	ftp_reply(sess, FTP_GREET, "(miniftpd 0.1)");
	int ret;
	while (1)
	{
		memset(sess->cmdline, 0, sizeof(sess->cmdline));
		memset(sess->cmd, 0, sizeof(sess->cmd));
		memset(sess->arg, 0, sizeof(sess->arg));
		ret = readline(sess->ctrl_fd, sess->cmdline, MAX_COMMAND_LINE);
		if (ret == -1)
			ERR_EXIT("readline");
		else if (ret == 0)
			exit(EXIT_SUCCESS);

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
		if (strcmp("USER", sess->cmd) == 0)
		{
			do_user(sess);
		}
		else if (strcmp("PASS", sess->cmd) == 0)
		{
			do_pass(sess);
		}
	}
}

//����+״̬+�ı�
void ftp_reply(session_t *sess, int status, const char *text)
{
	char buf[1024] = {0};
	sprintf(buf, "%d %s\r\n", status, text);
	writen(sess->ctrl_fd, buf, strlen(buf));
}

//static���棺�ú���ֻ�����ڵ�ǰģ��
static void do_user(session_t *sess)
{
	//USER jjl
	//getpwnam�����û��������Ի�ȡ������Ľṹ��
	struct passwd *pw = getpwnam(sess->arg);//man getpwnam,����cat /etc/passwd�ļ����Ӧ
	if (pw == NULL)//��ȡʧ��
	{
		// �û�������
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	sess->uid = pw->pw_uid;
	ftp_reply(sess, FTP_GIVEPWORD, "Please specify the password.");
	
}

static void do_pass(session_t *sess)
{
	// PASS 123456
	//getpwuid����uid�õ�passwd�ṹ��
	struct passwd *pw = getpwuid(sess->uid);//man getpwuid
	if (pw == NULL)
	{
		// �û�������
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	printf("name=[%s]\n", pw->pw_name);
	//getspnam�����û���������ȡӰ��passwd�ļ���Ϣ
	//pw->pw_name���û���
	struct spwd *sp = getspnam(pw->pw_name);//��ȡ/etc/passwd�ļ���Ϣ
	if (sp == NULL)
	{
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	// �����Ľ��м���
	//����crypt(Ҫ���ܵ����ģ�����)�������ܹ���������Ϊ���ӣ��õ�һ�����ܵ�����
	char *encrypted_pass = crypt(sess->arg, sp->sp_pwdp);
	// ��֤����
	if (strcmp(encrypted_pass, sp->sp_pwdp) != 0)
	{
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}
	
	//��¼�ɹ�����Ҫ��Ķ���������Ĳ������Դ�man getpwnam�л�ȡ��
	setegid(pw->pw_gid);
	seteuid(pw->pw_uid);
	chdir(pw->pw_dir);
	ftp_reply(sess, FTP_LOGINOK, "Login successful.");
}
