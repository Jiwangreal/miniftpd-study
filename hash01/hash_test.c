#include <stdio.h>

///假设哈希表的地址空间不超过101
#define BUCKETS 101

//依据字符串str得到一个哈希地址
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
	//将这些关键码经过哈希函数映射到哈希表中
	char *keywords[] =
	{
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
	};

	// 保存哈希表每个地址的映射次数
	// 0地址的映射次数用count[0]来表示，1地址的映射次数用count[1]来表示，。。。
	int count[BUCKETS];

	int i;
	for (i=0; i<BUCKETS; i++)
	{
		count[i] = 0;
	}

	//数组的个数=数组的大小/每一项的大小
	int size = sizeof(keywords) / sizeof(keywords[0]);

	for (i=0; i<size; i++)
	{	
			//虽然没有哈希表，但可以看到映射到哈希表的什么位置
			int pos = SDBMHash(keywords[i]);
			count[pos]++;//该映射位置的映射的次数++
	}

	for (i=0; i<size; i++)
	{
		int pos = SDBMHash(keywords[i]);
		printf("%-10s: %03d %d\n", keywords[i], pos, count[pos]);//-表示左对齐
	}

	return 0;
}
