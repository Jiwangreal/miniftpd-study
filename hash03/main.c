#include "hash.h"
#include "common.h"

typedef struct stu
{
	char sno[5];
	char name[32];
	int age;
}stu_t;

typedef struct stu2
{
	int sno;
	char name[32];
	int age;
}stu2_t;

//以字符串作为关键码，是一个字符串哈希函数
unsigned int hash_str(unsigned int buckets, void *key)
{
	char *sno = (char *)key;//依据字符串sno，计算出散列地址
	//下面是自己编写的字符串哈希函数
	unsigned int index = 0;

	while (*sno)
	{
		index = *sno + 4*index;
		sno++;
	}

	return index % buckets;//% buckets操作：保证散列的地址不会超过buckets桶的大小
}

//整数作为关键码
unsigned int hash_int(unsigned int buckets, void *key)
{
	int *sno = (int *)key;
	return (*sno) % buckets;
}

int main(void)
{
	/*
	stu_t stu_arr[] =
	{
		{ "1234", "AAAA", 20 },
		{ "4568", "BBBB", 23 },
		{ "6729", "AAAA", 19 }
	};

	hash_t *hash = hash_alloc(256, hash_str);

	int size = sizeof(stu_arr) / sizeof(stu_arr[0]);//是一个条目数组的大小，eg：{ "1234", "AAAA", 20 }
	int i;
	for (i=0; i<size; i++)
	{
		//当前的关键码用学号，strlen(stu_arr[i].sno)代表关键码的长度
		//&stu_arr[i]代表将学生记录作为数据项，sizeof(stu_arr[i])代表学生记录的大小
		hash_add_entry(hash, stu_arr[i].sno, strlen(stu_arr[i].sno),
			&stu_arr[i], sizeof(stu_arr[i]));
	}
	
	//查找一下
	//关键码是4568，strlen("4568")代表关键码的长度
	stu_t *s = (stu_t *)hash_lookup_entry(hash, "4568", strlen("4568"));
	if (s)
	{
		printf("%s %s %d\n", s->sno, s->name, s->age);
	}
	else
	{
		printf("not found\n");
	}
	
	//从哈希表中删除1234相关数据
	hash_free_entry(hash, "1234", strlen("1234"));
	s = (stu_t *)hash_lookup_entry(hash, "1234", strlen("1234"));
	if (s)
	{
		printf("%s %s %d\n", s->sno, s->name, s->age);
	}
	else
	{
		printf("not found\n");
	}
	*/

	stu2_t stu_arr[] =
	{
		{ 1234, "AAAA", 20 },
		{ 4568, "BBBB", 23 },
		{ 6729, "AAAA", 19 }
	};

	hash_t *hash = hash_alloc(256, hash_int);

	int size = sizeof(stu_arr) / sizeof(stu_arr[0]);
	int i;
	for (i=0; i<size; i++)
	{
		//由于封装的哈希表是无类型指针void*（第二个参数），所以可以是任意类型，接下来演示的是关键码是以恶搞整数
		//学号作为关键码
		hash_add_entry(hash, &(stu_arr[i].sno), sizeof(stu_arr[i].sno),//将sizeof(stu_arr[i].sno)写成sizeof(int)也可以，是四个字节的
			&stu_arr[i], sizeof(stu_arr[i]));
	}

	int sno = 4568;
	stu2_t *s = (stu2_t *)hash_lookup_entry(hash, &sno, sizeof(sno));
	if (s)
	{
		printf("%d %s %d\n", s->sno, s->name, s->age);
	}
	else
	{
		printf("not found\n");
	}
	
	sno = 1234;
	hash_free_entry(hash, &sno, sizeof(sno));
	s = (stu2_t *)hash_lookup_entry(hash, &sno, sizeof(sno));
	if (s)
	{
		printf("%d %s %d\n", s->sno, s->name, s->age);
	}
	else
	{
		printf("not found\n");
	}

	return 0;
}