#include <stdio.h>

///�����ϣ��ĵ�ַ�ռ䲻����101
#define BUCKETS 101

//�����ַ���str�õ�һ����ϣ��ַ
unsigned int SDBMHash(char *str)
{
    unsigned int hash = 0;
 
    while (*str)
    {
        // equivalent to: hash = 65599*hash + (*str++);
        hash = (*str++) + (hash << 6) + (hash << 16) - hash;
    }
 
    return (hash & 0x7FFFFFFF) % BUCKETS;
}

int main(void)
{
	//����Щ�ؼ��뾭����ϣ����ӳ�䵽��ϣ����
	char *keywords[] =
	{
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
	};

	// �����ϣ��ÿ����ַ��ӳ�����
	// 0��ַ��ӳ�������count[0]����ʾ��1��ַ��ӳ�������count[1]����ʾ��������
	int count[BUCKETS];

	int i;
	for (i=0; i<BUCKETS; i++)
	{
		count[i] = 0;
	}

	//����ĸ���=����Ĵ�С/ÿһ��Ĵ�С
	int size = sizeof(keywords) / sizeof(keywords[0]);

	for (i=0; i<size; i++)
	{	
			//��Ȼû�й�ϣ�������Կ���ӳ�䵽��ϣ���ʲôλ��
			int pos = SDBMHash(keywords[i]);
			count[pos]++;//��ӳ��λ�õ�ӳ��Ĵ���++
	}

	for (i=0; i<size; i++)
	{
		int pos = SDBMHash(keywords[i]);
		printf("%-10s: %03d %d\n", keywords[i], pos, count[pos]);//-��ʾ�����
	}

	return 0;
}
