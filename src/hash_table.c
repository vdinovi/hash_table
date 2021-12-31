#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include "hash_table.h"

static inline size_t hash(const char* key, const size_t capacity)
{
    size_t sum = 0;
    for (int i = 0; i < strlen(key); ++i) {
        sum += key[i];
    }
    return sum % capacity;
}

static inline int hash_table_entry_init(hash_table_entry_t* entry, const char* key, void* data, const size_t size)
{
    hash_table_entry_t temp = {0};
    strlcpy(temp.key, key, sizeof(temp.key));
    temp.size = size;

    void *dest = NULL;
    if (size <= HASH_TABLE_ENTRY_CACHE_BYTES) {
        dest = (void*)&temp.storage.cache;
    } else {
        dest = temp.storage.data = malloc(size);
        if (!temp.storage.data) {
            errno = ENOMEM;
            return 1;
        }
    }
    memcpy(dest, data, size);
    *entry = temp;
    return 0;
}

static inline void hash_table_entry_destroy(hash_table_entry_t* entry)
{
    if (entry->size > 0) {
        if (entry->size > HASH_TABLE_ENTRY_CACHE_BYTES) {
            free(entry->storage.data);
            entry->storage.data = NULL;
        } else {
            memset(entry->storage.cache, 0, sizeof(entry->storage.cache));
        }
    }
    entry->size = 0;
    entry->key[0] = 0;
}

static inline hash_table_chain_t* hash_table_add_link(hash_table_chain_t* prev)
{
    hash_table_chain_t *link = malloc(sizeof(hash_table_chain_t));
    if (!link) {
        errno = ENOMEM;
        return NULL;
    }
    link->next = prev->next;
    prev->next = link;
    return link;
}

int hash_table_init(hash_table_t* ht, const size_t capacity)
{
    assert(ht);

    hash_table_t temp = {
        .chains = NULL,
        .capacity = capacity,
        .size = 0,
    };
    temp.chains = malloc(sizeof(hash_table_chain_t) * capacity);
    if (!temp.chains) {
        errno = ENOMEM;
        return 1;
    }
    memset(temp.chains, 0, sizeof(hash_table_chain_t) * capacity);
    *ht = temp;

    return 0;
}

int hash_table_destroy(hash_table_t* ht)
{
    assert(ht);

    for (int i = 0; i < ht->capacity; i++) {
        hash_table_chain_t *iter = &ht->chains[i];
        while (iter) {
            hash_table_chain_t *temp = iter;
            hash_table_entry_destroy(&iter->entry);
            iter = iter->next;
            if (temp != &ht->chains[i]) {
                free(temp);
            }
        }
    }
    free(ht->chains);
    ht->chains = NULL;
    ht->size = 0;
    return 0;
}

int hash_table_set(hash_table_t* ht, const char* key, void* data, const size_t size)
{
    assert(ht);
    assert(key);
    assert(data);
    assert(size > 0);

    if (strlen(key) > HASH_TABLE_MAX_KEY_LENGTH) {
        errno = EINVAL;
        return 1;
    }

    hash_table_entry_t entry = {0};
    if (hash_table_entry_init(&entry, key, data, size)) {
        return 1;
    }

    size_t hash_value = hash(key, ht->capacity);
    hash_table_chain_t *iter = &ht->chains[hash_value];
    while (iter) {
        if (!iter->entry.size) {
            ht->size++;
            break;
        }
        if (!strncmp(iter->entry.key, key, HASH_TABLE_MAX_KEY_LENGTH)) {
            break;
        }
        if (!iter->next) {
            iter = hash_table_add_link(iter);
            if (!iter) {
                hash_table_entry_destroy(&entry);
                return 1;
            }
            ht->size++;
            break;
        }
        iter = iter->next;
    }

    iter->entry = entry;
    return 0;
}

hash_table_entry_t* hash_table_get_entry(hash_table_t* ht, const char* key)
{
    assert(ht);
    assert(key);

    if (strlen(key) > HASH_TABLE_MAX_KEY_LENGTH) {
        errno = EINVAL;
        return NULL;
    }

    size_t hash_value = hash(key, ht->capacity);
    hash_table_chain_t *iter = &ht->chains[hash_value];
    while (iter) {
        if (iter->entry.size > 0) {
            if (!strncmp(iter->entry.key, key, HASH_TABLE_MAX_KEY_LENGTH)) {
                return &iter->entry;
            }
        }
        iter = iter->next;
    }

    errno = ENODATA;
    return NULL;
}


void* hash_table_entry_get(hash_table_entry_t* entry)
{
    assert(entry);

    if (entry->size < HASH_TABLE_ENTRY_CACHE_BYTES) {
        return (void*)&entry->storage.cache;
    } else {
        return &entry->storage.data;
    }
}

int hash_table_delete(hash_table_t* ht, const char* key)
{
    assert(ht);
    assert(key);

    if (strlen(key) > HASH_TABLE_MAX_KEY_LENGTH) {
        errno = EINVAL;
        return 1;
    }

    size_t hash_value = hash(key, ht->capacity);
    hash_table_chain_t *prev = NULL;
    hash_table_chain_t *iter = &ht->chains[hash_value];
    while (iter) {
        if (iter->entry.size > 0) {
            if (!strncmp(iter->entry.key, key, HASH_TABLE_MAX_KEY_LENGTH)) {
                hash_table_entry_destroy(&iter->entry);
                if (prev) {
                    prev->next = iter->next;
                }
                if (iter != &ht->chains[hash_value]) {
                    free(iter);
                }
                ht->size--;
                return 0;
            }
        }
        prev = iter;
        iter = iter->next;
    }

    errno = ENODATA;
    return 1;
}


void hash_table_debug_print(hash_table_t* ht)
{
    assert(ht);

    printf("=== Hash Table { capacity = %zu, size = %zu } ===\n", ht->capacity, ht->size);
    for (int i = 0; i < ht->capacity; i++) {
        printf("%d:", i);
        hash_table_chain_t *iter = &ht->chains[i];
        while (iter) {
            printf("{ key = %s, size = %zu, ... } -> ", iter->entry.key, iter->entry.size);
            iter = iter->next;
        }
        printf("NULL\n");
    }
    printf("=============================================\n");
}



