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
	// 控制连接
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// 父子进程通道
	int parent_fd;
	int child_fd;
} session_t;
*/

extern session_t *p_sess;
static unsigned int s_children;//定义一个静态变量，表示当前的子进程的数目，静态变量的初始值等于0

void check_limits(session_t *sess);
void handle_sigchld(int sig);

int main(void)
{
	/*
	list_common();
	exit(EXIT_SUCCESS);
	*/

	// 字符串测试
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

	//char *str3 = "abcDef";		// 指针指向一个字符串常量，常量不能被修改
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
	// 控制连接
	uid_t uid;
	int ctrl_fd;
	char cmdline[MAX_COMMAND_LINE];
	char cmd[MAX_COMMAND];
	char arg[MAX_ARG];

	// 数据连接
	struct sockaddr_in *port_addr;
	int pasv_listen_fd;
	int data_fd;
	int data_process;

	// 限速
	unsigned int bw_upload_rate_max;
	unsigned int bw_download_rate_max;
	long bw_transfer_start_sec;
	long bw_transfer_start_usec;


	// 父子进程通道
	int parent_fd;
	int child_fd;

	// FTP协议状态
	int is_ascii;
	long long restart_pos;
	char *rnfr_name;
	int abor_received;

	// 连接数限制
	unsigned int num_clients;
} session_t;
*/

	session_t sess = 
	{
		/* 控制连接 */
		0, -1, "", "", "",
		/* 数据连接 */
		NULL, -1, -1, 0,
		/* 限速 */
		0, 0, 0, 0,
		/* 父子进程通道 */
		-1, -1,
		/* FTP协议状态 */
		0, 0, NULL, 0,
		/* 连接数限制 */
		0
	};

	p_sess = &sess;

	sess.bw_upload_rate_max = tunable_upload_max_rate;
	sess.bw_download_rate_max = tunable_download_max_rate;
	
	//当子进程退出时，会给父进程发送SIGCHLD信号，现在应该捕捉该信号，用于：子进程退出时，如何改变父进程的s_children变量的值
	signal(SIGCHLD, handle_sigchld);
	int listenfd = tcp_server(tunable_listen_address, tunable_listen_port);
	int conn;
	pid_t pid;

	while (1)
	{
		conn = accept_timeout(listenfd, NULL, 0);
		if (conn == -1)
			ERR_EXIT("accept_tinmeout");

		
		++s_children;//，创建成功++
		sess.num_clients = s_children;//连接数等于当前子进程的数目

		pid = fork();
		if (pid == -1)
		{
			--s_children;//创建失败了要--，因为在fork之前++了，所以失败了要--
			ERR_EXIT("fork");
		}

		//sess变量会被子进程继承下来，当fork完毕后，子进程会复制父进程的s_children整个变量
		if (pid == 0)
		{
			close(listenfd);
			sess.ctrl_fd = conn;
			check_limits(&sess);//连接数个数的判定
			signal(SIGCHLD, SIG_IGN);//在begin_session又创建了一个子进程：ftp服务进程，ftp服务进程退出的时候，nobody进程是
									//不需要完成handle_sigchld信号处理任务，但是信号也是会被继承下去的，所以不让它继承下去，可以在
									//这里重新signal一下，即：在子进程中，对子进程的子进程的退出的信号重新进行一次关联，当即：当ftp服务进程退出的时候，
									//向nobody进程发送SIGCHLD信号，该信号处理程序不会对s_children进行--操作，因为它进行这个操作是没有任何意义的，就算它进行
									//这个操作，对我们的程序是没有影响的
			begin_session(&sess);		
		}
		else
			close(conn);
	}
	return 0;
}

void check_limits(session_t *sess)
{
	if (tunable_max_clients > 0 && sess->num_clients > tunable_max_clients)//判断是否启用了tunable_max_clients限制以及 &&当前连接数 > 最大连接数
	{
		ftp_reply(sess, FTP_TOO_MANY_USERS, 
			"There are too many connected users, please try later.");
		//要使用miniftpd26\ftpproto.c中的ftp_reply函数，需要将其声明放到ftpproto.h头文件中，因为其定义在miniftpd26\ftpproto.c中中是私有的
		//注意还要包含ftpproto.h头文件
		//--s_children;注意这里更改的只是子进程的s_children变量，不能更改父进程的s_children变量，不要这么写
		exit(EXIT_FAILURE);//不应该开启会话，直接退出
	}
}

//避免僵尸进程的方法
void handle_sigchld(int sig)
{
	pid_t pid;
	//waitpid(等待的子进程的pid,等待子进程的退出状态,等待方式);
	//waitpid返回值是等待的子进程的pid
	//为啥使用while？因为当有很多个子进程同时退出的时候，有些信号可能没有办法立刻等待到
	while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)//-1表示等待任意的进程，NULL表示不需要关心子进程的退出状态
	{											//WNOHANG表示没有等待到子进程就立刻返回，并且waitpid会返回为0，表示没有等待到子进程
		;
	}

	--s_children;//改变父进程的s_children变量，父进程维护了一个子进程数目的变量
}

