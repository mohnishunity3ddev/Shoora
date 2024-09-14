#include "hashtable_strstr.h"
#include "hashtable.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: Hash Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static u32
HashFunc_StrInt_FNV1A(const void *In, size_t Length, u32 Seed, b32 IgnoreCase)
{
    u32 Result = Seed;

    u32 Character;
    size_t Index;

    for (Index = 0; Index < Length; ++Index)
    {
        Character = ((const unsigned char *)In)[Index];
        if (IgnoreCase)
        {
            Character = ToLower(Character);
        }

        Result ^= Character;
        Result *= 16777619; // FNV Prime.
    }

    return Result;
}

static u32
HashFunc_StrCaseSensitive_FNV1A(const void *In, u32 Seed)
{
    i32 len = StringLen((const char *)In) - 1;
    u32 Result = HashFunc_StrInt_FNV1A(In, len, Seed, false);
    return Result;
}

static u32
HashFunc_StrNoCaseSensitive_FNV1A(const void *In, u32 Seed)
{
    i32 len = StringLen((const char *)In) - 1;
    u32 Result = HashFunc_StrInt_FNV1A(In, len, Seed, true);
    return Result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: Equals Functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static b32
HashTable_StringEqualsFunc(const void *A, const void *B)
{
    b32 Result = StringCompare((const char *)A, (const char *)B);
    return Result;
}

static b32
HashTable_StringLowerCaseEqualsFunc(const void *A, const void *B)
{
    b32 Result = StringLowerCaseCompare((const char *)A, (const char *)B);
    return Result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: Everything Else
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

htable_strstr *
HashTableCreate_StrStr(unsigned int Flags)
{
    htable_hash HashFunction = HashFunc_StrCaseSensitive_FNV1A;
    htable_key_equals EqualsFunction = HashTable_StringEqualsFunc;
    // (NOTE: void *(*)(void *))StringDuplicateMalloc casts the function pointer as something
    // which returns a void * and takes in a void *.
    htable_callbacks Callbacks = {.KeyCopy   = (void *(*)(void *))StringDuplicateMalloc,
                                  .KeyFree   = free,
                                  .ValueCopy = (void *(*)(void *))StringDuplicateMalloc,
                                  .ValueFree = free};

    if (Flags & HTABLE_STR_NOCASECMP)
    {
        HashFunction = HashFunc_StrNoCaseSensitive_FNV1A;
        EqualsFunction = HashTable_StringLowerCaseEqualsFunc;
    }

    htable_strstr *Result = (htable_strstr *)HashTableCreate(HashFunction, EqualsFunction, &Callbacks);
    return Result;
}

void
HashTableDestroy_StrStr(htable_strstr *HashTable)
{
    HashTableDestroy((htable *)HashTable);
}

void
HashTableInsert_StrStr(htable_strstr *HashTable, const char *Key, const char *Value)
{
    HashTableInsert((htable *)HashTable, (void *)Key, (void *)Value);
}

void
HashTableRemove_StrStr(htable_strstr *HashTable, const char *Key)
{
    HashTableRemove((htable *)HashTable, (void *)Key);
}

b32
HashTableGet_StrStr(htable_strstr *HashTable, const char *Key, const char **Value)
{
    b32 Result = HashTableGet((htable *)HashTable, (void *)Key, (void **)Value);
    return Result;
}

const char *
HashTableGetDirect_StrStr(htable_strstr *HashTable, const char *Key)
{
    char *Result = (char *)HashTableGetDirect((htable *)HashTable, (void *)Key);
    return Result;
}

htable_strstr_enum *
HashTableEnumCreate_StrStr(htable_strstr *HashTable)
{
    htable_strstr_enum *Result = (htable_strstr_enum *)HashTableEnumCreate((htable *)HashTable);
    return Result;
}

b32
HashTableEnumNext_StrStr(htable_strstr_enum *HashEnum, const char **Key, const char **Value)
{
    b32 Result = HashTableEnumNext((htable_enum *)HashEnum, (void **)Key, (void **)Value);
    return Result;
}

void
HashTableEnumDestroy_StrStr(htable_strstr_enum *HashEnum)
{
    HashTableEnumDestroy((htable_enum *)HashEnum);
}

#include <platform/platform.h>
static void
TestHashTableStrStr()
{
    htable_strstr *h;
    htable_strstr_enum *he;
    const char *a = "xyz";
    const char *b;
    size_t num = 20;
    size_t i;
    char t1[64];
    char t2[64];

    h = HashTableCreate_StrStr();

    HashTableInsert_StrStr(h, "Abc", a);
    b = HashTableGetDirect_StrStr(h, "Abc");
    LogDebug("[Abc]:[%s] %p, %p.\n", b, a, b);

    HashTableInsert_StrStr(h, "aBc", "orange");
    b = HashTableGetDirect_StrStr(h, "Abc");
    LogDebug("[Abc]:%s %p, %p.\n", b, a, b);

    b = HashTableGetDirect_StrStr(h, "aBc");
    LogDebug("[aBc]:%s %p, %p.\n", b, a, b);

    for (i = 0; i < num; ++i)
    {
        Platform_GenerateString(t1, ARRAY_SIZE(t1), "a%zu", i);
        Platform_GenerateString(t2, ARRAY_SIZE(t2), "%zu", (i * 100) + i + (i / 2));
        HashTableInsert_StrStr(h, t1, t2);
    }

    he = HashTableEnumCreate_StrStr(h);
    while (HashTableEnumNext_StrStr(he, &a, &b))
    {
        LogDebug("Key = %s, Value = %s.\n", a, b);
    }
    HashTableEnumDestroy_StrStr(he);
    HashTableDestroy_StrStr(h);
}
