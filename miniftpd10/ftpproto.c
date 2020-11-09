#include "ftpproto.h"
#include "sysutil.h"
#include "str.h"
#include "ftpcodes.h"
#include "tunable.h"

void ftp_reply(session_t *sess, int status, const char *text);
void ftp_lreply(session_t *sess, int status, const char *text);

int list_common(session_t *sess);

int get_transfer_fd(session_t *sess);

static void do_user(session_t *sess);
static void do_pass(session_t *sess);
static void do_cwd(session_t *sess);
static void do_cdup(session_t *sess);
static void do_quit(session_t *sess);
static void do_port(session_t *sess);
static void do_pasv(session_t *sess);
static void do_type(session_t *sess);
static void do_stru(session_t *sess);
static void do_mode(session_t *sess);
static void do_retr(session_t *sess);
static void do_stor(session_t *sess);
static void do_appe(session_t *sess);
static void do_list(session_t *sess);
static void do_nlst(session_t *sess);
static void do_rest(session_t *sess);
static void do_abor(session_t *sess);
static void do_pwd(session_t *sess);
static void do_mkd(session_t *sess);
static void do_rmd(session_t *sess);
static void do_dele(session_t *sess);
static void do_rnfr(session_t *sess);
static void do_rnto(session_t *sess);
static void do_site(session_t *sess);
static void do_syst(session_t *sess);
static void do_feat(session_t *sess);
static void do_size(session_t *sess);
static void do_stat(session_t *sess);
static void do_noop(session_t *sess);
static void do_help(session_t *sess);

typedef struct ftpcmd
{
	const char *cmd;
	void (*cmd_handler)(session_t *sess);
} ftpcmd_t;


static ftpcmd_t ctrl_cmds[] = {
	/* 访问控制命令 */
	{"USER",	do_user	},
	{"PASS",	do_pass	},
	{"CWD",		do_cwd	},
	{"XCWD",	do_cwd	},
	{"CDUP",	do_cdup	},
	{"XCUP",	do_cdup	},
	{"QUIT",	do_quit	},
	{"ACCT",	NULL	},
	{"SMNT",	NULL	},
	{"REIN",	NULL	},
	/* 传输参数命令 */
	{"PORT",	do_port	},
	{"PASV",	do_pasv	},
	{"TYPE",	do_type	},
	{"STRU",	do_stru	},
	{"MODE",	do_mode	},

	/* 服务命令 */
	{"RETR",	do_retr	},
	{"STOR",	do_stor	},
	{"APPE",	do_appe	},
	{"LIST",	do_list	},
	{"NLST",	do_nlst	},
	{"REST",	do_rest	},
	{"ABOR",	do_abor	},
	{"\377\364\377\362ABOR", do_abor},
	{"PWD",		do_pwd	},
	{"XPWD",	do_pwd	},
	{"MKD",		do_mkd	},
	{"XMKD",	do_mkd	},
	{"RMD",		do_rmd	},
	{"XRMD",	do_rmd	},
	{"DELE",	do_dele	},
	{"RNFR",	do_rnfr	},
	{"RNTO",	do_rnto	},
	{"SITE",	do_site	},
	{"SYST",	do_syst	},
	{"FEAT",	do_feat },
	{"SIZE",	do_size	},
	{"STAT",	do_stat	},
	{"NOOP",	do_noop	},
	{"HELP",	do_help	},
	{"STOU",	NULL	},
	{"ALLO",	NULL	}
};



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
		// 去除\r\n
		str_trim_crlf(sess->cmdline);
		printf("cmdline=[%s]\n", sess->cmdline);
		// 解析FTP命令与参数
		str_split(sess->cmdline, sess->cmd, sess->arg, ' ');
		printf("cmd=[%s] arg=[%s]\n", sess->cmd, sess->arg);
		// 将命令转换为大写
		str_upper(sess->cmd);
		// 处理FTP命令
		/*
		if (strcmp("USER", sess->cmd) == 0)
		{
			do_user(sess);
		}
		else if (strcmp("PASS", sess->cmd) == 0)
		{
			do_pass(sess);
		}
		*/

		int i;
		int size = sizeof(ctrl_cmds) / sizeof(ctrl_cmds[0]);
		for (i=0; i<size; i++)
		{
			if (strcmp(ctrl_cmds[i].cmd, sess->cmd) == 0)
			{
				if (ctrl_cmds[i].cmd_handler != NULL)
				{
					ctrl_cmds[i].cmd_handler(sess);
				}
				else
				{
					ftp_reply(sess, FTP_COMMANDNOTIMPL, "Unimplement command.");
				}
				
				break;
			}
		}

		if (i == size)
		{
			ftp_reply(sess, FTP_BADCMD, "Unknown command.");
		}
	}
}

void ftp_reply(session_t *sess, int status, const char *text)
{
	char buf[1024] = {0};
	sprintf(buf, "%d %s\r\n", status, text);
	writen(sess->ctrl_fd, buf, strlen(buf));
}

void ftp_lreply(session_t *sess, int status, const char *text)
{
	char buf[1024] = {0};
	sprintf(buf, "%d-%s\r\n", status, text);
	writen(sess->ctrl_fd, buf, strlen(buf));
}

int list_common(session_t *sess)
{
	DIR *dir = opendir(".");
	if (dir == NULL)
	{
		return 0;
	}

	struct dirent *dt;
	struct stat sbuf;
	while ((dt = readdir(dir)) != NULL)
	{
		if (lstat(dt->d_name, &sbuf) < 0)
		{
			continue;
		}
		if (dt->d_name[0] == '.')
			continue;

		char perms[] = "----------";
		perms[0] = '?';

		mode_t mode = sbuf.st_mode;
		switch (mode & S_IFMT)
		{
		case S_IFREG:
			perms[0] = '-';
			break;
		case S_IFDIR:
			perms[0] = 'd';
			break;
		case S_IFLNK:
			perms[0] = 'l';
			break;
		case S_IFIFO:
			perms[0] = 'p';
			break;
		case S_IFSOCK:
			perms[0] = 's';
			break;
		case S_IFCHR:
			perms[0] = 'c';
			break;
		case S_IFBLK:
			perms[0] = 'b';
			break;
		}

		if (mode & S_IRUSR)
		{
			perms[1] = 'r';
		}
		if (mode & S_IWUSR)
		{
			perms[2] = 'w';
		}
		if (mode & S_IXUSR)
		{
			perms[3] = 'x';
		}
		if (mode & S_IRGRP)
		{
			perms[4] = 'r';
		}
		if (mode & S_IWGRP)
		{
			perms[5] = 'w';
		}
		if (mode & S_IXGRP)
		{
			perms[6] = 'x';
		}
		if (mode & S_IROTH)
		{
			perms[7] = 'r';
		}
		if (mode & S_IWOTH)
		{
			perms[8] = 'w';
		}
		if (mode & S_IXOTH)
		{
			perms[9] = 'x';
		}
		if (mode & S_ISUID)
		{
			perms[3] = (perms[3] == 'x') ? 's' : 'S';
		}
		if (mode & S_ISGID)
		{
			perms[6] = (perms[6] == 'x') ? 's' : 'S';
		}
		if (mode & S_ISVTX)
		{
			perms[9] = (perms[9] == 'x') ? 't' : 'T';
		}

		char buf[1024] = {0};
		int off = 0;
		off += sprintf(buf, "%s ", perms);
		off += sprintf(buf + off, " %3d %-8d %-8d ", sbuf.st_nlink, sbuf.st_uid, sbuf.st_gid);
		off += sprintf(buf + off, "%8lu ", (unsigned long)sbuf.st_size);

		const char *p_date_format = "%b %e %H:%M";
		struct timeval tv;
		gettimeofday(&tv, NULL);
		time_t local_time = tv.tv_sec;
		if (sbuf.st_mtime > local_time || (local_time - sbuf.st_mtime) > 60*60*24*182)
		{
			p_date_format = "%b %e  %Y";
		}

		char datebuf[64] = {0};
		struct tm* p_tm = localtime(&local_time);
		strftime(datebuf, sizeof(datebuf), p_date_format, p_tm);
		off += sprintf(buf + off, "%s ", datebuf);
		if (S_ISLNK(sbuf.st_mode))
		{
			char tmp[1024] = {0};
			readlink(dt->d_name, tmp, sizeof(tmp));
			off += sprintf(buf + off, "%s -> %s\r\n", dt->d_name, tmp);
		}
		else
		{
			off += sprintf(buf + off, "%s\r\n", dt->d_name);
		}


		//printf("%s", buf);
		writen(sess->data_fd, buf, strlen(buf));//将buf发送给客户端
	}

	closedir(dir);

	return 1;
}

int port_active(session_t *sess)
{
	if (sess->port_addr)
	{
		return 1;
	}

	return 0;
}

int pasv_active(session_t *sess)
{
	return 0;
}

int get_transfer_fd(session_t *sess)
{
	//printf("xxxxxxxxxxxxxxxxxxxxxxxxxx\n");//调式用
	// 检测是否收到PORT或者PASV命令
	if (!port_active(sess) && !pasv_active(sess))
	{
		//printf("yyyyyyyyyyyyyyyy\n");
		return 0;
	}
	//printf("mmmmmmmmmmmmmmmmmm\n");

	// 如果是主动模式
	if (port_active(sess))
	{
		//printf("aaaaaaaaaaaaaaaaaa\n");
		/*
		socket
		bind 20
		connect
		*/
		// tcp_client(20);//这里是没有办法bind20端口的，因为这是jjl用户，非root，需要nobody进程的协助
		//若使用tcp_client(20);，服务端会出现bind:Permission denied
		int fd = tcp_client(0);//暂且先不bind 20端口
		if (connect_timeout(fd, sess->port_addr, tunable_connect_timeout) < 0)
		{
			close(fd);
			return 0;
		}

//printf("bbbbbbbbbbbbbbbb\n");
		sess->data_fd = fd;
	}

	//port_addr用完以后最好判定一下是否是空指针，若不是NULL，说明前面做了do_port()分配了内存，此时没有用了，要将其free掉
	if (sess->port_addr)
	{
		free(sess->port_addr);
		sess->port_addr = NULL;
	}

	return 1;
}

static void do_user(session_t *sess)
{
	//USER jjl
	struct passwd *pw = getpwnam(sess->arg);
	if (pw == NULL)
	{
		// 用户不存在
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	sess->uid = pw->pw_uid;
	ftp_reply(sess, FTP_GIVEPWORD, "Please specify the password.");
	
}

static void do_pass(session_t *sess)
{
	// PASS 123456
	struct passwd *pw = getpwuid(sess->uid);
	if (pw == NULL)
	{
		// 用户不存在
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	printf("name=[%s]\n", pw->pw_name);
	struct spwd *sp = getspnam(pw->pw_name);
	if (sp == NULL)
	{
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	// 将明文进行加密
	char *encrypted_pass = crypt(sess->arg, sp->sp_pwdp);
	// 验证密码
	if (strcmp(encrypted_pass, sp->sp_pwdp) != 0)
	{
		ftp_reply(sess, FTP_LOGINERR, "Login incorrect.");
		return;
	}

	setegid(pw->pw_gid);
	seteuid(pw->pw_uid);
	chdir(pw->pw_dir);
	ftp_reply(sess, FTP_LOGINOK, "Login successful.");
}

static void do_cwd(session_t *sess)
{
}

static void do_cdup(session_t *sess)
{
}

static void do_quit(session_t *sess)
{
}

static void do_port(session_t *sess)
{
	//解析：PORT 192,168,0,100,123,233
	//123,233表示端口的高8位和低8位
	unsigned int v[6];

	//scanf表示从标准输入到变量中
	//sscanf按照一定的格式，格式化到相应的变量当中
	sscanf(sess->arg, "%u,%u,%u,%u,%u,%u", &v[2], &v[3], &v[4], &v[5], &v[0], &v[1]);
	sess->port_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));//注意这里是网际socket地址，不要用通用socket地址struct sockaddr
	memset(sess->port_addr, 0, sizeof(struct sockaddr_in));
	sess->port_addr->sin_family = AF_INET;
	unsigned char *p = (unsigned char *)&sess->port_addr->sin_port;
	//端口应该是2个字节的数据：&v[0], &v[1]
	p[0] = v[0];//一个字节
	p[1] = v[1];//一个字节

	p = (unsigned char *)&sess->port_addr->sin_addr;
	p[0] = v[2];
	p[1] = v[3];
	p[2] = v[4];
	p[3] = v[5];

	ftp_reply(sess, FTP_PORTOK, "PORT command successful. Consider using PASV.");
}

static void do_pasv(session_t *sess)
{
}

static void do_type(session_t *sess)
{
	if (strcmp(sess->arg, "A") == 0)
	{
		sess->is_ascii = 1;
		ftp_reply(sess, FTP_TYPEOK, "Switching to ASCII mode.");
	}
	else if (strcmp(sess->arg, "I") == 0)
	{
		sess->is_ascii = 0;
		ftp_reply(sess, FTP_TYPEOK, "Switching to Binary mode.");
	}
	else
	{
		ftp_reply(sess, FTP_BADCMD, "Unrecognised TYPE command.");
	}

}

static void do_stru(session_t *sess)
{
}

static void do_mode(session_t *sess)
{
}

static void do_retr(session_t *sess)
{
}

static void do_stor(session_t *sess)
{
}

static void do_appe(session_t *sess)
{
}

static void do_list(session_t *sess)
{
	// 创建数据连接:如果是port，就调用connect，如果是pasv，就调用accept
	if (get_transfer_fd(sess) == 0)
	{
		return;
	}
	// 150
	ftp_reply(sess, FTP_DATACONN, "Here comes the directory listing.");

	// 传输列表
	list_common(sess);
	// 关闭数据套接字
	close(sess->data_fd);//因为客户端会判断对等方的数据socket是否关闭，如果没关闭，客户端会一直接收，这会导致客户端无法显示接收的目录
	// 226
	ftp_reply(sess, FTP_TRANSFEROK, "Directory send OK.");


}

static void do_nlst(session_t *sess)
{
}

static void do_rest(session_t *sess)
{
}

static void do_abor(session_t *sess)
{
}

static void do_pwd(session_t *sess)
{
	char text[1024] = {0};
	char dir[1024+1] = {0};
	getcwd(dir, 1024);
	sprintf(text, "\"%s\"", dir);

	ftp_reply(sess, FTP_PWDOK, text);
}

static void do_mkd(session_t *sess)
{
}

static void do_rmd(session_t *sess)
{
}

static void do_dele(session_t *sess)
{
}

static void do_rnfr(session_t *sess)
{
}

static void do_rnto(session_t *sess)
{
}

static void do_site(session_t *sess)
{
}

static void do_syst(session_t *sess)
{
	ftp_reply(sess, FTP_SYSTOK, "UNIX Type: L8");
}

static void do_feat(session_t *sess)
{
	ftp_lreply(sess, FTP_FEAT, "Features:");
	writen(sess->ctrl_fd, " EPRT\r\n", strlen(" EPRT\r\n"));
	writen(sess->ctrl_fd, " EPSV\r\n", strlen(" EPSV"));
	writen(sess->ctrl_fd, " MDTM\r\n", strlen(" MDTM\r\n"));
	writen(sess->ctrl_fd, " PASV\r\n", strlen(" PASV\r\n"));
	writen(sess->ctrl_fd, " REST STREAM\r\n", strlen(" REST STREAM\r\n"));
	writen(sess->ctrl_fd, " SIZE\r\n", strlen(" SIZE\r\n"));
	writen(sess->ctrl_fd, " TVFS\r\n", strlen(" TVFS\r\n"));
	writen(sess->ctrl_fd, " UTF8\r\n", strlen(" UTF8\r\n"));
	ftp_reply(sess, FTP_FEAT, "End");
}

static void do_size(session_t *sess)
{
}

static void do_stat(session_t *sess)
{
}

static void do_noop(session_t *sess)
{
}

static void do_help(session_t *sess)
{
}

