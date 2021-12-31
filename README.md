Simple hash table implementation in c99 with no external dependencies outside of the c std lib


Warning: hack lib used in some of my personal projects. If you came across this looking for a C hash table
implementation, there are plenty of better options out there.

Only really tested on OSX 11

Make targets of note:
- `test` => build and run unit tests
- `build` => build dynamic library

Notes:
- Only accepts string keys of limited length
- Hash algorithm is simple string-sum + modulus
- No linear/quadratic probing
- Internal memory management, any value provided must be an lvalue


Example

```
    // Initialize the hash table
    size_t capacity = 5;
    hash_table_t ht;
    hash_table_init(&ht, capacity);
    
    // Set "AAA" => 7
    int i = 7;
    int status = hash_table_set(&ht, "AAA", (void*)&i, sizeof(int));
    // Fetch "AAA"
    hash_table_entry_t *entry = hash_table_get_entry(&ht, "AAA");
    printf("ht[%s] = %d\n", "AAA", *(int*)hash_table_entry_get(entry));
    
    // Set "BBB" => 3.14159
    float f = 3.14159;
    int status = hash_table_set(&ht, "BBB", (void*)&f, sizeof(float));
    // Fetch "BBB"
    hash_table_entry_t *entry = hash_table_get_entry(&ht, "AAA");
    printf("ht[%s] = %f\n", "BBB", *(float*)hash_table_entry_get(entry));
    
    // Replace "AAA" => "letters"
    char *str = "letters";
    int status = hash_table_set(&ht, "AAA", (void*)str, strlen(str) + 1);
    // Fetch "AAA"
    hash_table_entry_t *entry = hash_table_get_entry(&ht, "AAA");
    printf("ht[%s] = %s\n", "AAA", (char*)hash_table_entry_get(entry));
    
    // Delete "BBB"
    int status = hash_table_delete(&ht, "BBB");
    
    // Delete the hash table 
    hash_table_destroy(&ht);
```

