#include "common.h"
#include "sysutil.h"
#include "session.h"
#include "str.h"
#include "tunable.h"
#include "parseconf.h"
#include "ftpproto.h"
#include "ftpcodes.h"

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

extern session_t *p_sess;
static unsigned int s_children;//����һ����̬��������ʾ��ǰ���ӽ��̵���Ŀ����̬�����ĳ�ʼֵ����0

void check_limits(session_t *sess);
void handle_sigchld(int sig);

int main(void)
{
	/*
	list_common();
	exit(EXIT_SUCCESS);
	*/

	// �ַ�������
	/*
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

	//char *str3 = "abcDef";		// ָ��ָ��һ���ַ����������������ܱ��޸�
	char str3[] = "abcDef";
	str_upper(str3);
	printf("str3=%s\n", str3);

	long long result = str_to_longlong("12345678901234");
	printf("result = %lld\n", result);


	int n = str_octal_to_uint("711");
	printf("n=%d\n", n);
	*/

	parseconf_load_file(MINIFTP_CONF);

	printf("tunable_pasv_enable=%d\n", tunable_pasv_enable);
	printf("tunable_port_enable=%d\n", tunable_port_enable);

	printf("tunable_listen_port=%u\n", tunable_listen_port);
	printf("tunable_max_clients=%u\n", tunable_max_clients);
	printf("tunable_max_per_ip=%u\n", tunable_max_per_ip);
	printf("tunable_accept_timeout=%u\n", tunable_accept_timeout);
	printf("tunable_connect_timeout=%u\n", tunable_connect_timeout);
	printf("tunable_idle_session_timeout=%u\n", tunable_idle_session_timeout);
	printf("tunable_data_connection_timeout=%u\n", tunable_data_connection_timeout);
	printf("tunable_local_umask=0%o\n", tunable_local_umask);
	printf("tunable_upload_max_rate=%u\n", tunable_upload_max_rate);
	printf("tunable_download_max_rate=%u\n", tunable_download_max_rate);

	if (tunable_listen_address == NULL)
		printf("tunable_listen_address=NULL\n");
	else
		printf("tunable_listen_address=%s\n", tunable_listen_address);


	if (getuid() != 0)
	{
		fprintf(stderr, "miniftpd: must be started as root\n");
		exit(EXIT_FAILURE);
	}

/*
typedef struct session
{
	// ��������
	uid_t uid;
	int ctrl_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// ��������
	struct sockaddr_in *port_addr;
	int pasv_listen_fd;
	int data_fd;
	int data_process;

	// ����
	unsigned int bw_upload_rate_max;
	unsigned int bw_download_rate_max;
	long bw_transfer_start_sec;
	long bw_transfer_start_usec;


	// ���ӽ���ͨ��
	int parent_fd;
	int child_fd;

	// FTPЭ��״̬
	int is_ascii;
	long long restart_pos;
	char *rnfr_name;
	int abor_received;

	// ����������
	unsigned int num_clients;
} session_t;
*/

	session_t sess = 
	{
		/* �������� */
		0, -1, "", "", "",
		/* �������� */
		NULL, -1, -1, 0,
		/* ���� */
		0, 0, 0, 0,
		/* ���ӽ���ͨ�� */
		-1, -1,
		/* FTPЭ��״̬ */
		0, 0, NULL, 0,
		/* ���������� */
		0
	};

	p_sess = &sess;

	sess.bw_upload_rate_max = tunable_upload_max_rate;
	sess.bw_download_rate_max = tunable_download_max_rate;
	
	//���ӽ����˳�ʱ����������̷���SIGCHLD�źţ�����Ӧ�ò�׽���źţ����ڣ��ӽ����˳�ʱ����θı丸���̵�s_children������ֵ
	signal(SIGCHLD, handle_sigchld);
	int listenfd = tcp_server(tunable_listen_address, tunable_listen_port);
	int conn;
	pid_t pid;

	while (1)
	{
		conn = accept_timeout(listenfd, NULL, 0);
		if (conn == -1)
			ERR_EXIT("accept_tinmeout");

		
		++s_children;//�������ɹ�++
		sess.num_clients = s_children;//���������ڵ�ǰ�ӽ��̵���Ŀ

		pid = fork();
		if (pid == -1)
		{
			--s_children;//����ʧ����Ҫ--����Ϊ��fork֮ǰ++�ˣ�����ʧ����Ҫ--
			ERR_EXIT("fork");
		}

		//sess�����ᱻ�ӽ��̼̳���������fork��Ϻ��ӽ��̻Ḵ�Ƹ����̵�s_children��������
		if (pid == 0)
		{
			close(listenfd);
			sess.ctrl_fd = conn;
			check_limits(&sess);//�������������ж�
			signal(SIGCHLD, SIG_IGN);//��begin_session�ִ�����һ���ӽ��̣�ftp������̣�ftp��������˳���ʱ��nobody������
									//����Ҫ���handle_sigchld�źŴ������񣬵����ź�Ҳ�ǻᱻ�̳���ȥ�ģ����Բ������̳���ȥ��������
									//��������signalһ�£��������ӽ����У����ӽ��̵��ӽ��̵��˳����ź����½���һ�ι�������������ftp��������˳���ʱ��
									//��nobody���̷���SIGCHLD�źţ����źŴ�����򲻻��s_children����--��������Ϊ���������������û���κ�����ģ�����������
									//��������������ǵĳ�����û��Ӱ���
			begin_session(&sess);		
		}
		else
			close(conn);
	}
	return 0;
}

void check_limits(session_t *sess)
{
	if (tunable_max_clients > 0 && sess->num_clients > tunable_max_clients)//�ж��Ƿ�������tunable_max_clients�����Լ� &&��ǰ������ > ���������
	{
		ftp_reply(sess, FTP_TOO_MANY_USERS, 
			"There are too many connected users, please try later.");
		//Ҫʹ��miniftpd26\ftpproto.c�е�ftp_reply��������Ҫ���������ŵ�ftpproto.hͷ�ļ��У���Ϊ�䶨����miniftpd26\ftpproto.c������˽�е�
		//ע�⻹Ҫ����ftpproto.hͷ�ļ�
		//--s_children;ע��������ĵ�ֻ���ӽ��̵�s_children���������ܸ��ĸ����̵�s_children��������Ҫ��ôд
		exit(EXIT_FAILURE);//��Ӧ�ÿ����Ự��ֱ���˳�
	}
}

//���⽩ʬ���̵ķ���
void handle_sigchld(int sig)
{
	pid_t pid;
	//waitpid(�ȴ����ӽ��̵�pid,�ȴ��ӽ��̵��˳�״̬,�ȴ���ʽ);
	//waitpid����ֵ�ǵȴ����ӽ��̵�pid
	//Ϊɶʹ��while����Ϊ���кܶ���ӽ���ͬʱ�˳���ʱ����Щ�źſ���û�а취���̵ȴ���
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)//-1��ʾ�ȴ�����Ľ��̣�NULL��ʾ����Ҫ�����ӽ��̵��˳�״̬
	{											//WNOHANG��ʾû�еȴ����ӽ��̾����̷��أ�����waitpid�᷵��Ϊ0����ʾû�еȴ����ӽ���
		;
	}

	--s_children;//�ı丸���̵�s_children������������ά����һ���ӽ�����Ŀ�ı���
}

