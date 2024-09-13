#ifndef SHU_HASHTABLE_STRSTR_H
#define SHU_HASHTABLE_STRSTR_H

#include <defines.h>

struct htable_strstr;
struct htable_strstr_enum;

enum htable_strstr_flags
{
    HTABLE_STR_CASECMP = 0, // Case Sensitivity when Comparison
    HTABLE_STR_NOCASECMP // No Case Sensitive Comparison
};

htable_strstr *HashTableCreate_StrStr(unsigned int Flags = 0);
void HashTableDestroy_StrStr(htable_strstr *HashTable);

void HashTableInsert_StrStr(htable_strstr *HashTable, const char *Key, const char *Value);
void HashTableRemove_StrStr(htable_strstr *HashTable, const char *Key);

b32  HashTableGet_StrStr(htable_strstr *HashTable, const char *Key, const char **Value);
const char *HashTableGetDirect_StrStr(htable_strstr *HashTable, const char *Key);

htable_strstr_enum *HashTableEnumCreate_StrStr(htable_strstr *HashTable);
b32  HashTableEnumNext_StrStr(htable_strstr_enum *HashEnum, const char **Key, const char **Value);
void HashTableEnumDestroy_StrStr(htable_strstr_enum *HashEnum);

void TestHashTableStrStr();

#endif
