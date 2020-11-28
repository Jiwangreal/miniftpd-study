#include "hash.h"
#include "common.h"
#include <assert.h>


typedef enum entry_status
{
	EMPTY,
	ACTIVE,
	DELETED
} entry_status_t;

typedef struct hash_node
{
	enum entry_status status;//引入了节点的状态，因为再插入的时候要判定这个地址是否是空的
							//这个地址存放一个节点，如果节点是空的就可以放进来，所以需要状态
							//假设将Broad删除，然后我们要查找Blum，Blum这时映射到了1这个地址，发现
							//产生了冲突，要向下探查2这个地址，若做的是物理删除的话，那么此时就不能继续探查了，因为是空的
							//所以这里应该做逻辑删除，给node一个删除状态，方便能够继续向下搜索。直到能找到Blum
	void *key;
	void *value;
} hash_node_t;

/*
typedef struct hash_node
{
	void *key;
	void *value;
	struct hash_node *prev;//去掉了前驱和后继指针，因为这不是链地址法
	struct hash_node *next;
} hash_node_t;
*/

struct hash
{
	unsigned int buckets;
	hashfunc_t hash_func;
	hash_node_t *nodes;
};
/*
struct hash
{
	unsigned int buckets;
	hashfunc_t hash_func;
	hash_node_t **nodes;
};
*/

unsigned int hash_get_bucket(hash_t *hash, void *key);
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size);


hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func)
{
	hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
	//assert(hash != NULL);
	hash->buckets = buckets;
	hash->hash_func = hash_func;
	//int size = buckets * sizeof(hash_node_t *);
	int size = buckets * sizeof(hash_node_t);
	//hash->nodes = (hash_node_t **)malloc(size);
	hash->nodes = (hash_node_t *)malloc(size);
	memset(hash->nodes, 0, size);
	return hash;
}

//查找的过程
void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size)
{
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
	if (node == NULL)
	{
		return NULL;
	}

	return node->value;
}

//插入的过程
void hash_add_entry(hash_t *hash, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	if (hash_lookup_entry(hash, key, key_size))//插入一个表项，若已经存在的话
	{
		fprintf(stderr, "duplicate hash key\n");
		return;
	}

	unsigned int bucket = hash_get_bucket(hash, key);
	unsigned int i = bucket;
	while (hash->nodes[i].status == ACTIVE)//不是一个空桶的话，继续探查
	{
		i = (i + 1) % hash->buckets;
		if (i == bucket)
		{
			// 没找到，并且表满
			return;
		}
	}
	//到这里，说明找到一个空位置
	hash->nodes[i].status = ACTIVE;
	//将这个表项放到这个节点中
	if (hash->nodes[i].key)
	{
		free(hash->nodes[i].key);
	}
	hash->nodes[i].key = malloc(key_size);
	memcpy(hash->nodes[i].key, key, key_size);
	if (hash->nodes[i].value)
	{
		free(hash->nodes[i].value);
	}
	hash->nodes[i].value = malloc(value_size);
	memcpy(hash->nodes[i].value, value, value_size);

}

void hash_free_entry(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);
	if (node == NULL)
		return;

	// 逻辑删除
	node->status = DELETED;
}

unsigned int hash_get_bucket(hash_t *hash, void *key)
{
	unsigned int bucket = hash->hash_func(hash->buckets, key);
	if (bucket >= hash->buckets)
	{
		fprintf(stderr, "bad bucket lookup\n");
		exit(EXIT_FAILURE);
	}

	return bucket;
}

hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size)
{
	unsigned int bucket = hash_get_bucket(hash, key);//获得一个桶号码，链地址法返回的是指针的指针
	unsigned int i = bucket;
	while (hash->nodes[i].status != EMPTY && memcmp(key, hash->nodes[i].key, key_size) != 0)
	{
		i = (i + 1) % hash->buckets;//增量序列为(i + 1)
		if (i == bucket)		// 探测了一圈
		{
			// 没找到，并且表满
			return NULL;
		}
	}
	if (hash->nodes[i].status == ACTIVE)
	{
		return &(hash->nodes[i]);
	}

	// 如果运行到这里，说明i为空位

	return NULL;
}
