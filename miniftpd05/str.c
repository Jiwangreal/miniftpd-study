#include "str.h"
#include "common.h"

void str_trim_crlf(char *str)
{
	char *p = &str[strlen(str)-1];//指向最后一个字符
	while (*p == '\r' || *p == '\n')
		*p-- = '\0';

}

void str_split(const char *str , char *left, char *right, char c)
{
	char *p = strchr(str, c);
	if (p == NULL)//说明没有参数
		strcpy(left, str);
	else
	{
		strncpy(left, str, p-str);//空格前的字符串拷贝到left
		strcpy(right, p+1);//空格之后的字符串拷贝到right
	}
}

int str_all_space(const char *str)
{
	while (*str)
	{
		if (!isspace(*str))//只要一个不是空白字符就返回0
			return 0;
		str++;
	}
	return 1;//全都是空白字符返回1
}

void str_upper(char *str)
{
	while (*str)
	{
		*str = toupper(*str);//toupper它不是通过参数*str返回回来，而是通过返回值返回回来
		str++;
	}
}

long long str_to_longlong(const char *str)
{
	//return atoll(str);//方法1：可以直接使用atoll，但不是所有的系统都支持
	/*
	将一个字符串转换为整数
	12345678

	8*1 +
	7*10 +
	6*10*10+
	。。。
	*/

	//方法2：
/*	long long result = 0;
	long long mult = 1;
	unsigned int len = strlen(str);
	unsigned int i;

	if (len > 15)
		return 0;


	for (i=0; i<len; i++)
	{
		char ch = str[len-(i+1)];//最后一个字符，len-i-1,因为i从0开始
		long long val;
		if (ch < '0' || ch > '9')
			return 0;

		val = ch - '0';
		val *= mult;
		result += val;
		mult *= 10;
	}
	*/

	//方法3：for循环倒过来
	long long result = 0;
	long long mult = 1;
	unsigned int len = strlen(str);
	int i;//注意这里不要写成unsigned int i;一旦i=0再进行减操作，它永远不会=-1，i会变成最大数了，因为i是无符号int，所以for循环
		//会不断循环，直到越界为止，因为不在'0'-'9'之间，所以会一直返回0

	if (len > 15)
		return 0;

	for (i=len-1; i>=0; i--)//len-1是最后一个字符
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
	方法1：与str_to_longlong类似
	123456745

	5*1 + 
	4*8 +
	7*8*8 +

	方法2：从左边第一个非0数开始，从高位开始计算
	如果是10进制
	0*8+1
	1*10 + 2 = 12
	12*10 + 3 =123
	123*10 + 4 = 1234

	这里是8进制，所以将上面的10换成8即可
	0*8+1
	1*8 + 2 = 12
	12*8 + 3 =123
	123*8 + 4 = 1234
	*/
	while (*str)
	{
		int digit = *str;//将当前的位保存至数字
		if (!isdigit(digit) || digit > '7')
			break;

		if (digit != '0')
			seen_non_zero_digit = 1;

		if (seen_non_zero_digit)
		{
			//等价于:result = result << 3;
			result <<= 3;//左移3bit就是乘以8，移位运算比直接乘以8效率高
			result += (digit - '0');
		}
		str++;
	}
	return result;
}
