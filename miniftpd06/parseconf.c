#include "parseconf.h"
#include "common.h"
#include "tunable.h"
#include "str.h"

static struct parseconf_bool_setting
{
  const char *p_setting_name;
  int *p_variable;
}
parseconf_bool_array[] =
{
	{ "pasv_enable", &tunable_pasv_enable },
	{ "port_enable", &tunable_port_enable },
	{ NULL, NULL }
};



static struct parseconf_uint_setting
{
	const char *p_setting_name;
	unsigned int *p_variable;
}
parseconf_uint_array[] =
{
	{ "listen_port", &tunable_listen_port },
	{ "max_clients", &tunable_max_clients },
	{ "max_per_ip", &tunable_max_per_ip },
	{ "accept_timeout", &tunable_accept_timeout },
	{ "connect_timeout", &tunable_connect_timeout },
	{ "idle_session_timeout", &tunable_idle_session_timeout },
	{ "data_connection_timeout", &tunable_data_connection_timeout },
	{ "local_umask", &tunable_local_umask },
	{ "upload_max_rate", &tunable_upload_max_rate },
	{ "download_max_rate", &tunable_download_max_rate },
	{ NULL, NULL }
};


//结构体数组
static struct parseconf_str_setting
{
	const char *p_setting_name;
	const char **p_variable;
}
parseconf_str_array[] =
{
	{ "listen_address", &tunable_listen_address },
	{ NULL, NULL }
};


void parseconf_load_file(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (fp == NULL)
		ERR_EXIT("fopen");

	char setting_line[1024] = {0};
	while (fgets(setting_line, sizeof(setting_line), fp) != NULL)
	{
		//判断是否是合法的配置行
		if (strlen(setting_line) == 0
			|| setting_line[0] == '#'
			|| str_all_space(setting_line))
			continue;

		str_trim_crlf(setting_line);//去除\r\n
		parseconf_load_setting(setting_line);
		memset(setting_line, 0, sizeof(setting_line));
	}

	fclose(fp);
}


void parseconf_load_setting(const char *setting)//仅仅代表字符串指针指向的内容不能被改变，不代表setting指针不能偏移
{
	//去除左空格
	while (isspace(*setting))
		setting++;

	char key[128] ={0};
	char value[128] = {0};
	str_split(setting, key, value, '=');
	if (strlen(value) == 0)
	{
		fprintf(stderr, "mising value in config file for: %s\n", key);
		exit(EXIT_FAILURE);
	}

	//字符串类型的配置项
	{
		const struct parseconf_str_setting *p_str_setting = parseconf_str_array;
		while (p_str_setting->p_setting_name != NULL)
		{
			if (strcmp(key, p_str_setting->p_setting_name) == 0)
			{
				const char **p_cur_setting = p_str_setting->p_variable;
				//变量的地址：p_cur_setting，*p_cur_setting取出变量的内容
				if (*p_cur_setting)//判断指针是否是空的，如果已经有数据了，先free
					free((char*)*p_cur_setting);//char* 可以隐式的转换为void*，因为free(void* str)
												//而*p_cur_setting是const char*类型的，它转换成void*会有警告

				//因为value是局部变量，若指针指向局部变量，那么局部变量销毁掉时，该指针会成为野指针
				//strdup内部先申请内存，然后将value字符串拷贝到这块内存中，指针指向这块内存
				*p_cur_setting = strdup(value);
				return;//找到配置项，就return
			}

			p_str_setting++;//直到找到配置项即可
		}
	}

	//bool类型的配置项
	{
		const struct parseconf_bool_setting *p_bool_setting = parseconf_bool_array;
		while (p_bool_setting->p_setting_name != NULL)
		{
			if (strcmp(key, p_bool_setting->p_setting_name) == 0)
			{
				str_upper(value);
				if (strcmp(value, "YES") == 0
					|| strcmp(value, "TRUE") == 0
					|| strcmp(value, "1") == 0)
					*(p_bool_setting->p_variable) = 1;
				else if (strcmp(value, "NO") == 0
					|| strcmp(value, "FALSE") == 0
					|| strcmp(value, "0") == 0)
					*(p_bool_setting->p_variable) = 0;
				else
				{
					fprintf(stderr, "bad bool value in config file for: %s\n", key);
					exit(EXIT_FAILURE);
				}

				return;
			}

			p_bool_setting++;
		}
	}

	//整数类型的配置项
	{
		const struct parseconf_uint_setting *p_uint_setting = parseconf_uint_array;
		while (p_uint_setting->p_setting_name != NULL)
		{
			if (strcmp(key, p_uint_setting->p_setting_name) == 0)
			{
				if (value[0] == '0')//说明是0开头的8进制
					*(p_uint_setting->p_variable) = str_octal_to_uint(value);
				else
					*(p_uint_setting->p_variable) = atoi(value);

				return;
			}

			p_uint_setting++;
		}
	}
}


