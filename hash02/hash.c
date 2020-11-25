#include "hash.h"
#include "common.h"
#include <assert.h>

//����һ���ڵ�����ݽṹ
typedef struct hash_node
{
	//������ָ��void*��ʾ���ؼ��������������������
	void *key;//��������ݽṹ����Ĺؼ���
	void *value;//��������ݽṹ����Ĺؼ�������Ӧ��������
	struct hash_node *prev;
	struct hash_node *next;
} hash_node_t;


struct hash
{
	unsigned int buckets;//Ͱ�ĸ���
	hashfunc_t hash_func;//��ϣ����
	hash_node_t **nodes;//��ϣ������ŵ�����ĵ�ַ�����Ի���Ҫ���������нڵ�����ݽṹ�����Ի�Ҫ��һ�������нڵ�����ݽṹ
						//�ؼ���ĵ�ַ�����������У���ָ���ָ�뱣�����һϵ�������ͷָ��
};

hash_node_t** hash_get_bucket(hash_t *hash, void *key);
hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size);


hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func)
{
	hash_t *hash = (hash_t *)malloc(sizeof(hash_t));
	//assert(hash != NULL);
	hash->buckets = buckets;//Ͱ�Ĵ�С
	hash->hash_func = hash_func;//��ϣ����
	int size = buckets * sizeof(hash_node_t *);//�ܹ���Ҫ�����ָ��ĸ���
	hash->nodes = (hash_node_t **)malloc(size);
	memset(hash->nodes, 0, size);//���ڴ���0
	return hash;
}

//���ҷ���
//�����ؼ����ҵ���Ӧ��������
//��������ؼ�����Ҫӳ��һ����ַ������˵5�����ַ��ӳ�䵽�õ�ַ֮����Ҫ�������н��в��ң�
//�ڲ��ҹ�������Ҫ�ȽϹؼ��룬�ҵ�����Ӧ�Ľڵ㣬Ȼ�󷵻�������Ӧ��������value
void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size)
{
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);//�ҵ�����Ӧ�Ľڵ�
	if (node == NULL)
	{
		return NULL;
	}

	return node->value;//�ҵ��˽ڵ㣬���ؽڵ�����Ӧ��������
}

void hash_add_entry(hash_t *hash, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	if (hash_lookup_entry(hash, key, key_size))//�ж��������Ƿ��Ѿ�����
	{
		fprintf(stderr, "duplicate hash key\n");
		return;
	}

	//���һ���µĽڵ�
	hash_node_t *node = malloc(sizeof(hash_node_t));
	node->prev = NULL;
	node->next = NULL;

	node->key = malloc(key_size);
	memcpy(node->key, key, key_size);

	node->value = malloc(value_size);
	memcpy(node->value, value, value_size);

	hash_node_t **bucket = hash_get_bucket(hash, key);
	if (*bucket == NULL)//*bucket˵����������
	{
		*bucket = node;
	}
	else
	{
		// ���½����뵽����ͷ����ͷ�巨��
		node->next = *bucket;//node�ڵ�ĺ�̣���Ӧ��1��
		(*bucket)->prev = node;//*bucket�ڵ��ǰ������Ӧ��2��
		*bucket = node;//��Ӧ��3��
	}

}

//�ͷ�һ���ڵ�
void hash_free_entry(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t *node = hash_get_node_by_key(hash, key, key_size);//�ҵ�����ڵ�
	if (node == NULL)
	{
		return;
	}

	free(node->key);//ɾ��֮ǰ���Ƚ�node��key��value��free
	free(node->value);

	//ɾ�������޷Ǿ�������ڵ��ָ���ƶ�
	//��֤ɾ���Ľڵ㲻��ͷ�ڵ㣬��Ϊ���ɾ���Ľڵ���ͷ�ڵ㣬ͷ�ڵ���ǰ��
	if (node->prev)
		node->prev->next = node->next;
	else//ɾ������ͷ�ڵ�Ļ�
	{
		hash_node_t **bucket = hash_get_bucket(hash, key);
		*bucket = node->next;
	}
	//��֤ɾ���Ľڵ㲻�����һ���ڵ㣬��Ϊ���ɾ���Ľڵ������һ���ڵ㣬�Ͳ�����������һ���ڵ��ǰ��ָ������ǰһ���ڵ�
	if (node->next)
		node->next->prev = node->prev;

	free(node);//ɾ���ڵ㱾����ڴ�

}

hash_node_t** hash_get_bucket(hash_t *hash, void *key)
{
	unsigned int bucket = hash->hash_func(hash->buckets, key);//���ݹ�ϣ�����õ�Ͱ�ţ���ϣ���������洫����
	if (bucket >= hash->buckets)
	{
		fprintf(stderr, "bad bucket lookup\n");
		exit(EXIT_FAILURE);
	}

	return &(hash->nodes[bucket]);//��ʵ��Ҫ�õ�����������ŵĵ�ַ����Ϊ�ÿռ�����ŵĵ�ַ��ָ��һ���ڵ�node��ָ��
								//nodes[bucket]���ָ��ָ���������ͷ�ڵ㣬Ȼ��������Ҫ�������ָ��ĵ�ַ
}


hash_node_t* hash_get_node_by_key(hash_t *hash, void *key, unsigned int key_size)
{
	hash_node_t **bucket = hash_get_bucket(hash, key);//���ݹؼ����ȡ������ͷָ��ĵ�ַ
	hash_node_t *node = *bucket;//*bucket��������ͷָ��
	if (node == NULL)//��ͷָ���ǿգ��򷵻�
	{
		return NULL;
	}

	//��ͷָ�벻ΪNULL������Ҫ�������н��б���
	while (node != NULL && memcmp(node->key, key, key_size) != 0)//������0˵��û���ҵ�
	{
		node = node->next;
	}

	return node;
}
