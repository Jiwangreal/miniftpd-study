#include "hash.h"
#include "common.h"
#include <assert.h>

//链表一个节点的数据结构
typedef struct hash_node
{
	//无类型指针void*表示：关键码和数据项都可以是任意的
	void *key;//链表的数据结构保存的关键码
	void *value;//链表的数据结构保存的关键码所对应的数据项
	struct hash_node *prev;
	struct hash_node *next;
} hash_node_t;


struct hash
{
	unsigned int buckets;//桶的个数
	hashfunc_t hash_func;//哈希函数
	hash_node_t **nodes;//哈希表所存放的链表的地址，所以还需要定义链表中节点的数据结构，所以还要顶一个链表中节点的数据结构
						//关键码的地址保存在数组中，该指针的指针保存的是一系列链表的头指针
};

hash_node_t** hash_get_bucket(hash_t *hash, void *key);
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size);


hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func)
{
	hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
	//assert(hash != NULL);
	hash->buckets = buckets;//桶的大小
	hash->hash_func = hash_func;//哈希函数
	int size = buckets * sizeof(hash_node_t *);//总共需要保存的指针的个数
	hash->nodes = (hash_node_t **)malloc(size);
	memset(hash->nodes, 0, size);//将内存清0
	return hash;
}

//查找方法
//给定关键码找到对应的数据项
//比如给定关键码需要映射一个地址，比如说5这个地址，映射到该地址之后还需要在链表中进行查找，
//在查找过程中需要比较关键码，找到所对应的节点，然后返回他所对应的数据项value
void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size)
{
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);//找到所对应的节点
	if (node == NULL)
	{
		return NULL;
	}

	return node->value;//找到了节点，返回节点所对应的数据项
}

void hash_add_entry(hash_t *hash, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	if (hash_lookup_entry(hash, key, key_size))//判断数据项是否已经存在
	{
		fprintf(stderr, "duplicate hash key\n");
		return;
	}

	//添加一个新的节点
	hash_node_t *node = malloc(sizeof(hash_node_t));
	node->prev = NULL;
	node->next = NULL;

	node->key = malloc(key_size);
	memcpy(node->key, key, key_size);

	node->value = malloc(value_size);
	memcpy(node->value, value, value_size);

	hash_node_t **bucket = hash_get_bucket(hash, key);
	if (*bucket == NULL)//*bucket说明链表不存在
	{
		*bucket = node;
	}
	else
	{
		// 将新结点插入到链表头部（头插法）
		node->next = *bucket;//node节点的后继，对应（1）
		(*bucket)->prev = node;//*bucket节点的前驱，对应（2）
		*bucket = node;//对应（3）
	}

}

//释放一个节点
void hash_free_entry(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);//找到这个节点
	if (node == NULL)
	{
		return;
	}

	free(node->key);//删除之前，先将node的key和value先free
	free(node->value);

	//删除操作无非就是链表节点的指针移动
	//保证删除的节点不是头节点，因为如果删除的节点是头节点，头节点无前驱
	if (node->prev)
		node->prev->next = node->next;
	else//删除的是头节点的话
	{
		hash_node_t **bucket = hash_get_bucket(hash, key);
		*bucket = node->next;
	}
	//保证删除的节点不是最后一个节点，因为如果删除的节点是最后一个节点，就不存在他的下一个节点的前驱指向它的前一个节点
	if (node->next)
		node->next->prev = node->prev;

	free(node);//删除节点本身的内存

}

hash_node_t** hash_get_bucket(hash_t *hash, void *key)
{
	unsigned int bucket = hash->hash_func(hash->buckets, key);//依据哈希函数得到桶号，哈希函数由外面传进来
	if (bucket >= hash->buckets)
	{
		fprintf(stderr, "bad bucket lookup\n");
		exit(EXIT_FAILURE);
	}

	return &(hash->nodes[bucket]);//其实需要得到数组中所存放的地址，因为该空间所存放的地址是指向一个节点node的指针
								//nodes[bucket]这个指针指向了链表的头节点，然后我们需要返回这个指针的地址
}


hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t **bucket = hash_get_bucket(hash, key);//依据关键码获取了链表头指针的地址
	hash_node_t *node = *bucket;//*bucket就是链表头指针
	if (node == NULL)//若头指针是空，则返回
	{
		return NULL;
	}

	//若头指针不为NULL，则需要在链表中进行遍历
	while (node != NULL && memcmp(node->key, key, key_size) != 0)//不等于0说明没有找到
	{
		node = node->next;
	}

	return node;
}
