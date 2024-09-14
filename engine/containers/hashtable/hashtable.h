#ifndef SHU_HASHTABLE_H
#define SHU_HASHTABLE_H

#include <defines.h>

struct htable;
struct htable_enum;

typedef u32     (*htable_hash)(const void *In, u32 Seed);
typedef void *  (*htable_key_copy)(void *In);
typedef b32     (*htable_key_equals)(const void *A, const void *B);
typedef void    (*htable_key_free)(void *In);
typedef void *  (*htable_value_copy)(void *In);
typedef void    (*htable_value_free)(void *In);

struct htable_callbacks
{
    htable_key_copy   KeyCopy;
    htable_key_free   KeyFree;
    htable_value_copy ValueCopy;
    htable_value_free ValueFree;
};

htable *HashTableCreate(htable_hash HashFunction, htable_key_equals KeyEqualsFunction, htable_callbacks *Callbacks);
void    HashTableDestroy(htable *HashTable);

void    HashTableInsert(htable *HashTable, void *Key, void *Value);
void    HashTableRemove(htable *HashTable, void *Key);

b32     HashTableGet(htable *HashTable, void *Key, void **Value);
void *  HashTableGetDirect(htable *HashTable, void *Key);

htable_enum *HashTableEnumCreate(htable *HashTable);
b32     HashTableEnumNext(htable_enum *HashEnum, void **Key, void **Value);
void    HashTableEnumDestroy(htable_enum *HashEnum);

#endif