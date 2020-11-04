#include "str.h"
#include "common.h"

void str_trim_crlf(char *str)
{
	char *p = &str[strlen(str)-1];//ָ�����һ���ַ�
	while (*p == '\r' || *p == '\n')
		*p-- = '\0';

}

void str_split(const char *str , char *left, char *right, char c)
{
	char *p = strchr(str, c);
	if (p == NULL)//˵��û�в���
		strcpy(left, str);
	else
	{
		strncpy(left, str, p-str);//�ո�ǰ���ַ���������left
		strcpy(right, p+1);//�ո�֮����ַ���������right
	}
}

int str_all_space(const char *str)
{
	while (*str)
	{
		if (!isspace(*str))//ֻҪһ�����ǿհ��ַ��ͷ���0
			return 0;
		str++;
	}
	return 1;//ȫ���ǿհ��ַ�����1
}

void str_upper(char *str)
{
	while (*str)
	{
		*str = toupper(*str);//toupper������ͨ������*str���ػ���������ͨ������ֵ���ػ���
		str++;
	}
}

long long str_to_longlong(const char *str)
{
	//return atoll(str);//����1������ֱ��ʹ��atoll�����������е�ϵͳ��֧��
	/*
	��һ���ַ���ת��Ϊ����
	12345678

	8*1 +
	7*10 +
	6*10*10+
	������
	*/

	//����2��
/*	long long result = 0;
	long long mult = 1;
	unsigned int len = strlen(str);
	unsigned int i;

	if (len > 15)
		return 0;


	for (i=0; i<len; i++)
	{
		char ch = str[len-(i+1)];//���һ���ַ���len-i-1,��Ϊi��0��ʼ
		long long val;
		if (ch < '0' || ch > '9')
			return 0;

		val = ch - '0';
		val *= mult;
		result += val;
		mult *= 10;
	}
	*/

	//����3��forѭ��������
	long long result = 0;
	long long mult = 1;
	unsigned int len = strlen(str);
	int i;//ע�����ﲻҪд��unsigned int i;һ��i=0�ٽ��м�����������Զ����=-1��i����������ˣ���Ϊi���޷���int������forѭ��
		//�᲻��ѭ����ֱ��Խ��Ϊֹ����Ϊ����'0'-'9'֮�䣬���Ի�һֱ����0

	if (len > 15)
		return 0;

	for (i=len-1; i>=0; i--)//len-1�����һ���ַ�
	{
		char ch = str[i];
		long long val;
		if (ch < '0' || ch > '9')
			return 0;

		val = ch - '0';
		val *= mult;
		result += val;
		mult *= 10;
	}

	return result;
}

unsigned int str_octal_to_uint(const char *str)
{
	unsigned int result = 0;
	int seen_non_zero_digit = 0;
	/*
	����1����str_to_longlong����
	123456745

	5*1 + 
	4*8 +
	7*8*8 +

	����2������ߵ�һ����0����ʼ���Ӹ�λ��ʼ����
	�����10����
	0*8+1
	1*10 + 2 = 12
	12*10 + 3 =123
	123*10 + 4 = 1234

	������8���ƣ����Խ������10����8����
	0*8+1
	1*8 + 2 = 12
	12*8 + 3 =123
	123*8 + 4 = 1234
	*/
	while (*str)
	{
		int digit = *str;//����ǰ��λ����������
		if (!isdigit(digit) || digit > '7')
			break;

		if (digit != '0')
			seen_non_zero_digit = 1;

		if (seen_non_zero_digit)
		{
			//�ȼ���:result = result << 3;
			result <<= 3;//����3bit���ǳ���8����λ�����ֱ�ӳ���8Ч�ʸ�
			result += (digit - '0');
		}
		str++;
	}
	return result;
}
