#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "unity.h"

#include "hash_table.h"

static void assert_hash_table_empty(hash_table_t* ht)
{
    TEST_ASSERT_NOT_NULL(ht);
    for (int i = 0; i < ht->capacity; ++i) {
        hash_table_chain_t *chain = &ht->chains[i];
        TEST_ASSERT_NULL(chain->next);
        TEST_ASSERT_EQUAL_UINT(0, chain->entry.size);
        TEST_ASSERT_EMPTY(chain->entry.key);
        TEST_ASSERT_EMPTY(chain->entry.storage.cache);
        TEST_ASSERT_NULL(chain->entry.storage.data);
    }
}

typedef struct test_struct_t {
    int n;
    char *str;
} test_struct_t;

static int v1 = 1;
static float v2 = 1.234;
static char v3[5] = "1234";
static test_struct_t v4 = { .n = 7, .str = "XYZ" };

void* test_data[4] = {
    (void*)&v1,
    (void*)&v2,
    (void*)v3,
    (void*)&v4,
};

size_t test_data_lengths[4] = {
    sizeof(v1),
    sizeof(v2),
    sizeof(v3),
    sizeof(v4),
};

static void seed_hash_table(hash_table_t* ht, char* keys[], size_t count)
{
    for (int i = 0; i < count; i++) {
        hash_table_set(ht, keys[i], test_data[i % 4], test_data_lengths[i % 4]);
    }
}

static char long_key[HASH_TABLE_MAX_KEY_LENGTH + 2];

void setUp()
{
    srand(time(NULL));

    memset(long_key, 'A', HASH_TABLE_MAX_KEY_LENGTH + 1);
    long_key[HASH_TABLE_MAX_KEY_LENGTH + 1] = 0;
}

void tearDown()
{

}

void test_hash_table_init_should_initializeTheHashTable()
{
    hash_table_t ht;

    int status = hash_table_init(&ht, 50);
    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(50, ht.capacity);
    TEST_ASSERT_EQUAL_UINT(0, ht.size);
    TEST_ASSERT_NOT_NULL(ht.chains);

    assert_hash_table_empty(&ht);
}

void test_hash_table_destroy_should_destroyTheHashTable()
{
    hash_table_t ht;

    hash_table_init(&ht, 3);
    TEST_ASSERT_EQUAL_UINT(0, ht.size);

    char* keys[5] = { "AAA", "BBB", "CCC", "DDD", "EEE" };
    seed_hash_table(&ht, keys, 5);
    TEST_ASSERT_EQUAL_UINT(5, ht.size);

    hash_table_destroy(&ht);
    TEST_ASSERT_EQUAL_UINT(0, ht.size);
    TEST_ASSERT_NULL(ht.chains);
}

void test_hash_table_set_should_insertAnEntryInTheHashTable()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    int v1 = 1;
    int status = hash_table_set(&ht, "AAA", (void*)&v1, sizeof(v1));
    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(1, ht.size);
    hash_table_entry_t *entry = hash_table_get_entry(&ht, "AAA");
    TEST_ASSERT_EQUAL_INT(v1, *(int*)hash_table_entry_get(entry));
}

void test_hash_table_set_should_replaceAnExistingEntryInTheHashTable()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    int v1 = 1;
    int status = hash_table_set(&ht, "AAA", (void*)&v1, sizeof(v1));
    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(1, ht.size);
    hash_table_entry_t *entry = hash_table_get_entry(&ht, "AAA");
    TEST_ASSERT_EQUAL_INT(v1, *(int*)hash_table_entry_get(entry));

    int v2 = 2;
    status = hash_table_set(&ht, "AAA", (void*)&v2, sizeof(v2));
    TEST_ASSERT_EQUAL_INT(0, status);
    TEST_ASSERT_EQUAL_UINT(1, ht.size);
    entry = hash_table_get_entry(&ht, "AAA");
    TEST_ASSERT_EQUAL_INT(v2, *(int*)hash_table_entry_get(entry));
}

void test_hash_table_set_should_returnErrorWhenKeyIsTooLong()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    int v1 = 1;
    int status = hash_table_set(&ht, long_key, (void*)&v1, sizeof(v1));
    TEST_ASSERT_EQUAL_INT(1, status);
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

void test_hash_table_get_entry_should_returnAnEntryFromTheHashTable()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    char* keys[5] = { "AAA", "BBB", "CCC", "DDD", "EEE" };
    seed_hash_table(&ht, keys, 5);
    TEST_ASSERT_EQUAL_UINT(5, ht.size);

    for (int i = 0; i < 5; ++i) {
        hash_table_entry_t *entry = hash_table_get_entry(&ht, keys[i]);
        TEST_ASSERT_EQUAL_MEMORY(test_data[i % 4], hash_table_entry_get(entry), test_data_lengths[i % 4]);
    }
}

void test_hash_table_get_entry_should_returnErrorWhenKeyIsTooLong()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    hash_table_entry_t *entry = hash_table_get_entry(&ht, long_key);
    TEST_ASSERT_NULL(entry);
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

void test_hash_table_delete_should_returnErrorWhenKeyIsTooLong()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    int status = hash_table_delete(&ht, long_key);
    TEST_ASSERT_EQUAL_INT(1, status);
    TEST_ASSERT_EQUAL_INT(EINVAL, errno);
}

void test_hash_table_delete_should_removeAnEntryFromTheHashTable()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    int status;
    char* keys[5] = { "AAA", "BBB", "CCC", "DDD", "EEE" };
    seed_hash_table(&ht, keys, 5);
    TEST_ASSERT_EQUAL_UINT(5, ht.size);

    for (int i = 0; i < 5; ++i) {
        TEST_ASSERT_EQUAL_UINT(5 - i, ht.size);
        status = hash_table_delete(&ht, keys[i]);
        TEST_ASSERT_EQUAL_INT(0, status);
    }
    TEST_ASSERT_EQUAL_UINT(0, ht.size);
}

void test_hash_table_delete_should_returnErrorWhenKeyNotFound()
{
    hash_table_t ht;
    hash_table_init(&ht, 3);

    int status = hash_table_delete(&ht, "AAA");
    TEST_ASSERT_EQUAL_INT(1, status);
    TEST_ASSERT_EQUAL_INT(ENODATA, errno);
}

int main()
{
    UNITY_BEGIN();
    RUN_TEST(test_hash_table_init_should_initializeTheHashTable);
    RUN_TEST(test_hash_table_destroy_should_destroyTheHashTable);
    RUN_TEST(test_hash_table_set_should_insertAnEntryInTheHashTable);
    RUN_TEST(test_hash_table_set_should_replaceAnExistingEntryInTheHashTable);
    RUN_TEST(test_hash_table_set_should_returnErrorWhenKeyIsTooLong);
    RUN_TEST(test_hash_table_get_entry_should_returnAnEntryFromTheHashTable);
    RUN_TEST(test_hash_table_get_entry_should_returnErrorWhenKeyIsTooLong);
    RUN_TEST(test_hash_table_delete_should_returnErrorWhenKeyIsTooLong);
    RUN_TEST(test_hash_table_delete_should_removeAnEntryFromTheHashTable);
    RUN_TEST(test_hash_table_delete_should_returnErrorWhenKeyNotFound);
    return UNITY_END();
}
