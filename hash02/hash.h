#ifndef _HASH_H_
#define _HASH_H_

typedef struct hash hash_t;//由于不想把struct hash结构体暴露在外面，所以将这个结构体放在,c文件中，而不放在.h头文件中，
							//那么这里就需要一个typedef
typedef unsigned int (*hashfunc_t)(unsigned int, void*);//（桶大小，关键码），返回值是关键码的地址

hash_t* hash_alloc(unsigned int buckets, hashfunc_t hash_func);
void* hash_lookup_entry(hash_t *hash, void* key, unsigned int key_size);
void hash_add_entry(hash_t *hash, void *key, unsigned int key_size,
	void *value, unsigned int value_size);
void hash_free_entry(hash_t *hash, void *key, unsigned int key_size);


#endif /* _HASH_H_ */

