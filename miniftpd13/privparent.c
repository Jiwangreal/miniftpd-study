#include "privparent.h"
#include "privsock.h"
#include "sysutil.h"
#include "tunable.h"

static void privop_pasv_get_data_sock(session_t *sess);
static void privop_pasv_active(session_t *sess);
static void privop_pasv_listen(session_t *sess);
static void privop_pasv_accept(session_t *sess);

//由于加了头文件，但是编译的时候还报：implicit declaration of function 'capset'，
//capset是原始的一个内核接口，所以我们自己定义capset接口
//这里capset调用了原始的内核接口
int capset(cap_user_header_t hdrp, const cap_user_data_t datap)
{
	return syscall(__NR_capset, hdrp, datap);//man syscall
	//syscal(系统调用号码,...可变参数)，capset系统调用的号码可以查看sys/syscall.h头文件
	//进去之后继续打开bits/syscall.h头文件，可以看到capset的系统调用号#define SYS_capset __NR_capset 
}

//给nobody进程一些必要的权限
void minimize_privilege(void)
{
	struct passwd *pw = getpwnam("nobody");
	if (pw == NULL)
		return;

	if (setegid(pw->pw_gid) < 0)
		ERR_EXIT("setegid");
	if (seteuid(pw->pw_uid) < 0)
		ERR_EXIT("seteuid");

	/*
	capabilities：将超级用户的特权划分成一些截然不同的单元。将这些单元称之为capabilities，即
	这些capabilities具有一些特殊的权限，这些capabilities能够被单独的开启或者关闭
	man 7 capabilities

	增加权限的方法：capset函数，man 2 capset
	让nobody进程具有bind特权端口的权限
	*/

	struct __user_cap_header_struct cap_header;//需要一个head
	struct __user_cap_data_struct cap_data;//需要一个data

	memset(&cap_header, 0, sizeof(cap_header));
	memset(&cap_data, 0, sizeof(cap_data));

	//32bit的系统选择_LINUX_CAPABILITY_VERSION_1，64bit的系统选择_LINUX_CAPABILITY_VERSION_2
	cap_header.version = _LINUX_CAPABILITY_VERSION_1;
	cap_header.pid = 0;//capset不需要进程号，如果是capget则需要，他是获取对应进程的capabilities

	__u32 cap_mask = 0;//__u32：这是无符号32bit整数，有32bit，首先先将他清0
	//cap_mask是一个掩码，里面可以存放很多capabilities
	cap_mask |= (1 << CAP_NET_BIND_SERVICE);
	//#include <linux/capability.h>,vi /usr/include/linux/capability.h，可以看到CAP_NET_BIND_SERVICE处于第10位
	/*
	....00000000000000000000000
	|或上
	....00000000000100000000000，左移第10位，就是1的后面有10个0
	*/

	//cap_data可以指定这些capabilities，通常指定的是同样的值
	//effective含义：应该具有什么样的capabilities？应该具有可以bind特权端口的capabilities：CAP_NET_BIND_SERVICE
	//permitted的含义：permitted中的capabilities集可以赋予effective，即effective的capabilities集合小于permitted集合，
	//permitted的含义：permitted的capabilities集合可以放到inheritable，是否可以被继承
	//调用exec系列函数的时候，这些capabilities可以被新进程继承，exec表示用新进程来替换
	cap_data.effective = cap_data.permitted = cap_mask;
	cap_data.inheritable = 0;//0表示不需要被新进程继承

	capset(&cap_header, &cap_data);
}

void handle_parent(session_t *sess)
{
	minimize_privilege();

	char cmd;
	while (1)
	{
		//read(sess->parent_fd, &cmd, 1);
		cmd = priv_sock_get_cmd(sess->parent_fd);
		// 解析内部命令
		// 处理内部命令
		switch (cmd)
		{
		case PRIV_SOCK_GET_DATA_SOCK:
			privop_pasv_get_data_sock(sess);
			break;
		case PRIV_SOCK_PASV_ACTIVE:
			privop_pasv_active(sess);
			break;
		case PRIV_SOCK_PASV_LISTEN:
			privop_pasv_listen(sess);
			break;
		case PRIV_SOCK_PASV_ACCEPT:
			privop_pasv_accept(sess);
			break;
		
		}
	}
}

static void privop_pasv_get_data_sock(session_t *sess)
{
	/*
	nobody进程接收PRIV_SOCK_GET_DATA_SOCK命令
进一步接收一个整数，也就是port
接收一个字符串，也就是ip

fd = socket 
bind(20)
connect(ip, port);

OK
send_fd
BAD
*/
	unsigned short port = (unsigned short)priv_sock_get_int(sess->parent_fd);
	char ip[16] = {0};
	priv_sock_recv_buf(sess->parent_fd, ip, sizeof(ip));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip);//inet_addr：点分十进制转换为32bit的整数

	int fd = tcp_client(20);
	if (fd == -1)
	{
		priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_BAD);
		return;
	}
	if (connect_timeout(fd, &addr, tunable_connect_timeout) < 0)
	{
		close(fd);
		priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_BAD);
		return;
	}

	priv_sock_send_result(sess->parent_fd, PRIV_SOCK_RESULT_OK);
	priv_sock_send_fd(sess->parent_fd, fd);
	close(fd);//nobody进程与客户端不直接进行通信，只是协助数据连接通道的建立而已
}

static void privop_pasv_active(session_t *sess)
{
}

static void privop_pasv_listen(session_t *sess)
{
}

static void privop_pasv_accept(session_t *sess)
{
}

