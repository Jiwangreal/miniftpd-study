#include "privparent.h"
#include "privsock.h"
#include "sysutil.h"
#include "tunable.h"

static void privop_pasv_get_data_sock(session_t *sess);
static void privop_pasv_active(session_t *sess);
static void privop_pasv_listen(session_t *sess);
static void privop_pasv_accept(session_t *sess);

//���ڼ���ͷ�ļ������Ǳ����ʱ�򻹱���implicit declaration of function 'capset'��
//capset��ԭʼ��һ���ں˽ӿڣ����������Լ�����capset�ӿ�
//����capset������ԭʼ���ں˽ӿ�
int capset(cap_user_header_t hdrp, const cap_user_data_t datap)
{
	return syscall(__NR_capset, hdrp, datap);//man syscall
	//syscal(ϵͳ���ú���,...�ɱ����)��capsetϵͳ���õĺ�����Բ鿴sys/syscall.hͷ�ļ�
	//��ȥ֮�������bits/syscall.hͷ�ļ������Կ���capset��ϵͳ���ú�#define SYS_capset __NR_capset 
}

//��nobody����һЩ��Ҫ��Ȩ��
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
	capabilities���������û�����Ȩ���ֳ�һЩ��Ȼ��ͬ�ĵ�Ԫ������Щ��Ԫ��֮Ϊcapabilities����
	��Щcapabilities����һЩ�����Ȩ�ޣ���Щcapabilities�ܹ��������Ŀ������߹ر�
	man 7 capabilities

	����Ȩ�޵ķ�����capset������man 2 capset
	��nobody���̾���bind��Ȩ�˿ڵ�Ȩ��
	*/

	struct __user_cap_header_struct cap_header;//��Ҫһ��head
	struct __user_cap_data_struct cap_data;//��Ҫһ��data

	memset(&cap_header, 0, sizeof(cap_header));
	memset(&cap_data, 0, sizeof(cap_data));

	//32bit��ϵͳѡ��_LINUX_CAPABILITY_VERSION_1��64bit��ϵͳѡ��_LINUX_CAPABILITY_VERSION_2
	cap_header.version = _LINUX_CAPABILITY_VERSION_1;
	cap_header.pid = 0;//capset����Ҫ���̺ţ������capget����Ҫ�����ǻ�ȡ��Ӧ���̵�capabilities

	__u32 cap_mask = 0;//__u32�������޷���32bit��������32bit�������Ƚ�����0
	//cap_mask��һ�����룬������Դ�źܶ�capabilities
	cap_mask |= (1 << CAP_NET_BIND_SERVICE);
	//#include <linux/capability.h>,vi /usr/include/linux/capability.h�����Կ���CAP_NET_BIND_SERVICE���ڵ�10λ
	/*
	....00000000000000000000000
	|����
	....00000000000100000000000�����Ƶ�10λ������1�ĺ�����10��0
	*/

	//cap_data����ָ����Щcapabilities��ͨ��ָ������ͬ����ֵ
	//effective���壺Ӧ�þ���ʲô����capabilities��Ӧ�þ��п���bind��Ȩ�˿ڵ�capabilities��CAP_NET_BIND_SERVICE
	//permitted�ĺ��壺permitted�е�capabilities�����Ը���effective����effective��capabilities����С��permitted���ϣ�
	//permitted�ĺ��壺permitted��capabilities���Ͽ��Էŵ�inheritable���Ƿ���Ա��̳�
	//����execϵ�к�����ʱ����Щcapabilities���Ա��½��̼̳У�exec��ʾ���½������滻
	cap_data.effective = cap_data.permitted = cap_mask;
	cap_data.inheritable = 0;//0��ʾ����Ҫ���½��̼̳�

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
		// �����ڲ�����
		// �����ڲ�����
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
	nobody���̽���PRIV_SOCK_GET_DATA_SOCK����
��һ������һ��������Ҳ����port
����һ���ַ�����Ҳ����ip

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
	addr.sin_addr.s_addr = inet_addr(ip);//inet_addr�����ʮ����ת��Ϊ32bit������

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
	close(fd);//nobody������ͻ��˲�ֱ�ӽ���ͨ�ţ�ֻ��Э����������ͨ���Ľ�������
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

