#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include <stdlib.h>

#define HASH_TABLE_DEFAULT_CAPACITY 1024
#define HASH_TABLE_ENTRY_CACHE_BYTES 32
#define HASH_TABLE_MAX_KEY_LENGTH 512

typedef union hash_table_entry_storage_u {
    char cache[HASH_TABLE_ENTRY_CACHE_BYTES];
    void* data;
} hash_table_entry_storage_u;

typedef struct hash_table_entry_t {
    char key[HASH_TABLE_MAX_KEY_LENGTH + 1];
    size_t size;
    hash_table_entry_storage_u storage;
} hash_table_entry_t;

typedef struct hash_table_chain_t {
    hash_table_entry_t entry;
    struct hash_table_chain_t *next;
} hash_table_chain_t;

typedef struct hash_table_t {
    hash_table_chain_t *chains;
    size_t capacity;
    size_t size;
} hash_table_t;

int hash_table_init(hash_table_t* ht, size_t capacity);
int hash_table_destroy(hash_table_t* ht);
int hash_table_set(hash_table_t* ht, const char* key, void* data, const size_t size);
hash_table_entry_t* hash_table_get_entry(hash_table_t* ht, const char* key);
int hash_table_delete(hash_table_t* ht, const char* key);
void* hash_table_entry_get(hash_table_entry_t* entry);
void hash_table_debug_print(hash_table_t* ht);

#endif /* __HASH_TABLE_H__ */
