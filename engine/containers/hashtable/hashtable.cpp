#include "hashtable.h"
#include <platform/platform.h>

struct htable_bucket
{
    void *Key;
    void *Value;
    struct htable_bucket *Next;
};

struct htable
{
    htable_hash HashFunc;
    htable_key_equals KeyEqualsFunc;
    htable_callbacks Callbacks;
    htable_bucket *Buckets;
    size_t NumBuckets;
    size_t NumUsed;
    u32 Seed;
};

struct htable_enum
{
    htable *HashTable;
    htable_bucket *CurrentBucket;
    size_t Index;
};

static const size_t BUCKET_START = 16;

// Default Callbacks
static void *HashTablePassthroughCopy   (void *V) { return V; }
static void  HashTablePassthroughDestroy(void *V) { return; }

// Hashing
static size_t
HashTableBucketIndex(htable *HashTable, void *Key)
{
    size_t Result = HashTable->HashFunc(Key, HashTable->Seed) % HashTable->NumBuckets;
    return Result;
}

// NOTE: Add a key-value pair to the hash table.
// The IsRehash boolean tells the function whether this function was called when we decided to rehash the table(
// meaning creating a new hashtable because the current is full.)
// You will notice throughout the code, we check if (!IsRehash) since we want to create copies of the key and value
// pairs if we are NOT rehashing. If we are rehashing, the copy is already there in the old hashtable so, we dont
// want to create new copies of the key value pairs.
static void
HashTableAddToBucket(htable *HashTable, void *Key, void *Value, b32 IsRehash)
{
    htable_bucket *Current;
    htable_bucket *End;

    size_t Index = HashTableBucketIndex(HashTable, Key);

    // Does the current bucket has something in it already?
    // In such case, add the key and value to the bucket.
    if (HashTable->Buckets[Index].Key == nullptr)
    {
        // If not rehashing, create copies of the key and value.
        if (!IsRehash)
        {
            Key = HashTable->Callbacks.KeyCopy(Key);
            if (Value != nullptr)
            {
                Value = HashTable->Callbacks.ValueCopy(Value);
            }
        }
        // Add the key-value pair to the bucket.
        HashTable->Buckets[Index].Key = Key;
        HashTable->Buckets[Index].Value = Value;

        // Increment the count of the used buckets if not rehashing.
        if (!IsRehash)
        {
            ++HashTable->NumUsed;
        }
    }
    else
    {
        // Collision.
        // There's already a key in the bucket for the current hashed index.
        End = HashTable->Buckets + Index;
        Current = HashTable->Buckets + Index;
        // Traverse the chain for adding to the current bucket index.
        do
        {
            // If the Key sent in here is the same as the key present in the current chain node,
            // replace its value since that is what is implicit in this case.
            if (HashTable->KeyEqualsFunc(Key, Current->Key))
            {
                // Free the Value since we want to replace it.
                if (Current->Value != nullptr) {
                    HashTable->Callbacks.ValueFree(Current->Value);
                }
                // Create a copy if not rehashing.
                if (!IsRehash && Value != nullptr) {
                    Value = HashTable->Callbacks.ValueCopy(Value);
                }
                // Update the value.
                Current->Value = Value;
                End = nullptr;
                break;
            }
            End = Current;
            Current = Current->Next;
        } while (Current != nullptr);

        // If we found that the key was not present when we were traversing the current bucket's list of chained
        // nodes.
        if (End != nullptr)
        {
            // get memory for the new bucket.
            Current = (htable_bucket *)calloc(1, sizeof(*Current->Next));
            // if not rehashing, copy the key and value.
            if (!IsRehash)
            {
                Key = HashTable->Callbacks.KeyCopy(Key);
                if (Value != nullptr) {
                    Value = HashTable->Callbacks.ValueCopy(Value);
                }
            }
            // Set the key and value for this new bucket. And set pointers for the linked list.
            Current->Key = Key;
            Current->Value = Value;
            End->Next = Current;
            // Increment the count of used buckets if not rehashing.
            if (!IsRehash) {
                ++HashTable->NumUsed;
            }
        }
    }
}

// Create a new hashtable since the current one is too full and will cause further chaining due to unavailability
// of empty buckets.
static void
HashTableRehash(htable *HashTable)
{
    htable_bucket *Buckets;
    htable_bucket *Current;
    htable_bucket *Next;
    size_t NumBuckets;
    size_t i;

    // We also want to make sure that if collisions are happening often(leading to creation of chains), we want to
    // grow the number of buckets in the hash table. The threshold for growing the bucket array is when it is 75%
    // full. Hard Limit for the length of bucket array is 1 << 31. If we have more items to add, we can just chain
    // them.
    // NOTE: Dont want to rehash if we have hit the mentioned thresholds.
    // NOTE: We are taking the max size threshold to be 1<<31 since we are doubling the amount of buckets every
    // time we rehash. So if its already more than this limit, doubling it will rollover to a smaller value which
    // is a problem.
    if (((HashTable->NumUsed + 1) < (size_t)(HashTable->NumBuckets * 0.75f)) ||
        (HashTable->NumBuckets >= 1 << 31))
    {
        return;
    }

    // Caching the old size and buckets memory
    NumBuckets = HashTable->NumBuckets;
    Buckets = HashTable->Buckets;

    // Setting new memory for the new enlarged hashtable.
    HashTable->NumBuckets <<= 1; // Increase the size of the hashtable.
    HashTable->Buckets = (htable_bucket *)calloc(HashTable->NumBuckets, sizeof(*Buckets));

    // Traversing the old hashtable.
    for (i = 0; i < NumBuckets; ++i)
    {
        if (Buckets[i].Key == nullptr) {
            continue;
        }

        HashTableAddToBucket(HashTable, Buckets[i].Key, Buckets[i].Value, true);
        if (Buckets[i].Next != nullptr)
        {
            Current = Buckets[i].Next;
            do
            {
                HashTableAddToBucket(HashTable, Current->Key, Current->Value, true);
                Next = Current->Next;
                free(Current);
                Current = Next;
            } while (Current != nullptr);
        }
    }

    // Free the old hashtable buckets.
    free(Buckets);
}


// Create a new hashtable.
static htable *
HashTableCreate(htable_hash HashFunc, htable_key_equals KeyEqualityFunc, htable_callbacks *Callbacks)
{
    htable *HashTable;

    if (HashFunc == nullptr || KeyEqualityFunc == nullptr) {
        return nullptr;
    }

    HashTable = (htable *)calloc(1, sizeof(*HashTable));
    HashTable->HashFunc = HashFunc;
    HashTable->KeyEqualsFunc = KeyEqualityFunc;

    // Set Default callback functions
    HashTable->Callbacks.KeyCopy   = HashTablePassthroughCopy;
    HashTable->Callbacks.KeyFree   = HashTablePassthroughDestroy;
    HashTable->Callbacks.ValueCopy = HashTablePassthroughCopy;
    HashTable->Callbacks.ValueFree = HashTablePassthroughDestroy;
    if (Callbacks != nullptr)
    {
        if (Callbacks->KeyCopy   != nullptr) HashTable->Callbacks.KeyCopy   = Callbacks->KeyCopy;
        if (Callbacks->KeyFree   != nullptr) HashTable->Callbacks.KeyFree   = Callbacks->KeyFree;
        if (Callbacks->ValueCopy != nullptr) HashTable->Callbacks.ValueCopy = Callbacks->ValueCopy;
        if (Callbacks->ValueFree != nullptr) HashTable->Callbacks.ValueFree = Callbacks->ValueFree;
    }

    HashTable->NumBuckets = BUCKET_START;
    HashTable->Buckets = (htable_bucket *)calloc(BUCKET_START, sizeof(*HashTable->Buckets));

    HashTable->Seed = Platform_GetRandomSeed();
    HashTable->Seed ^= ((u32)((u64)HashTableCreate << 16) & 0xFFFFFFFF) | (u32)((u64)HashTable & 0xFFFFFFFF);
    HashTable->Seed ^= (u32)((u64)&HashTable->Seed & 0xFFFFFFFF);

    return HashTable;
}

static void
HashTableDestroy(htable *HashTable)
{
    htable_bucket *Next;
    htable_bucket *Current;
    size_t i;

    if (HashTable == nullptr) { return; }

    for (i32 i = 0; i < HashTable->NumBuckets; ++i)
    {
        if (HashTable->Buckets[i].Key == nullptr)
            continue;

        // Free the first key-value pair in the current bucket.
        HashTable->Callbacks.KeyFree(HashTable->Buckets[i].Key);
        HashTable->Callbacks.ValueFree(HashTable->Buckets[i].Value);

        // Free the whole chain in the current bucket.
        Next = HashTable->Buckets[i].Next;
        while (Next != nullptr)
        {
            Current = Next;
            HashTable->Callbacks.KeyFree(Current->Key);
            HashTable->Callbacks.ValueFree(Current->Value);
            Next = Current->Next;
            free(Current);
        }
    }

    // Free the whole bucket array memory.
    free(HashTable->Buckets);
    // Free the hashtable struct memory.
    free(HashTable);
}

static void
HashTableInsert(htable *HashTable, void *Key, void *Value)
{
    void *CKey;
    void *CValue;

    if (HashTable == nullptr || Key == nullptr)
        return;

    // The Rehash function checks if the table is too full. If it is, it sets up a new hashtable with the old-data.
    // If its not, then it just returns.
    HashTableRehash(HashTable);
    HashTableAddToBucket(HashTable, Key, Value, false);
}

void
HashTableRemove(htable *HashTable, void *Key)
{
    htable_bucket *Current;
    htable_bucket *End;
    size_t Index;

    if (HashTable == nullptr || Key == nullptr)
        return;

    Index = HashTableBucketIndex(HashTable, Key);
    if (HashTable->Buckets[Index].Key == nullptr)
        return;

    // Is the key, the first chain node in the bucket?
    if (HashTable->KeyEqualsFunc(HashTable->Buckets[Index].Key, Key))
    {
        // Free the key-value pair.
        HashTable->Callbacks.KeyFree(HashTable->Buckets[Index].Key);
        HashTable->Callbacks.ValueFree(HashTable->Buckets[Index].Value);
        HashTable->Buckets[Index].Key = nullptr;

        Current = HashTable->Buckets[Index].Next;
        // Since this was the first node in the chain. Checking to see if the chain in the current bucket has more
        // than one element. if so, set the second node in the chain as the "HEAD" of the chain linked list(which
        // is the main bucket). Also, Free the memory held by the second node since that data has been moved to the
        // main node in the bucket(the "HEAD"), Also set the pointers to maintain the chain linked list.
        if (Current != nullptr)
        {
            HashTable->Buckets[Index].Key = Current->Key;
            HashTable->Buckets[Index].Value = Current->Value;
            HashTable->Buckets[Index].Next = Current->Next;
            // This was the memory held by the second node in the chain, next to the main bucket. Since the data
            // here has been moved to the main bucket memory, this has to be freed since we dont use it anymore.
            free(Current);
        }

        // Decrement the number of items in the hashtable.
        --HashTable->NumUsed;
        return;
    }

    // There is no matching key in the main node. Checking to see if its there in the chain.
    End = HashTable->Buckets + Index;
    Current = End->Next;
    while (Current != nullptr)
    {
        if (HashTable->KeyEqualsFunc(Key, Current->Key))
        {
            // Removing this current node from the chain linked list.
            End->Next = Current->Next;
            HashTable->Callbacks.KeyFree(Current->Key);
            HashTable->Callbacks.ValueFree(Current->Value);
            free(Current);
            // Decrement the number of items in the hashtable.
            --HashTable->NumUsed;
            break;
        }
        // Go to the next item in the chain.
        End = Current;
        Current = Current->Next;
    }
}

// NOTE: Get the Value out of the hashtable for the given Key.
static b32
HashTableGet(htable *HashTable, void *Key, void **Value)
{
    htable_bucket *Current;
    size_t Index;

    if (HashTable == nullptr || Key == nullptr)
        return false;

    // Get the Index for the Key provided.
    Index = HashTableBucketIndex(HashTable, Key);
    if (HashTable->Buckets[Index].Key == nullptr)
        return false;
    
    // Traverse the bucket and its chain looking for the matching key.
    Current = HashTable->Buckets + Index;
    while (Current != nullptr)
    {
        if (HashTable->KeyEqualsFunc(Key, Current->Key))
        {
            if (Value != nullptr)
            {
                *Value = Current->Value;
            }
            // Return true regardless of whether Value is available or not. This function can also be used if the
            // key exists.
            return true;
        }
        Current = Current->Next;
    }

    return false;
}

static void *
HashTableGetDirect(htable *HashTable, void *Key)
{
    void *Value = nullptr;
    HashTableGet(HashTable, Key, &Value);
    return Value;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// NOTE: Enumerate the HashTable
static htable_enum *
HashTableEnumCreate(htable *HashTable)
{
    htable_enum *HashTableEnum;

    if (HashTable == nullptr)
        return nullptr;

    HashTableEnum = (htable_enum *)calloc(1, sizeof(*HashTableEnum));
    HashTableEnum->HashTable = HashTable;

    return HashTableEnum;
}

static b32
HashTableEnumNext(htable_enum *HashEnum, void **Key, void **Value)
{
    void *MyKey = nullptr;
    void *MyValue = nullptr;

    if (HashEnum == nullptr || HashEnum->Index >= HashEnum->HashTable->NumBuckets)
        return false;

    if (Key == nullptr) Key = &MyKey;
    if (Value == nullptr) Value = &MyValue;

    // Current Bucket will be nullptr at the beginning of the enumeration, or we have traversed the whole chain of
    // a bucket till the end.
    if (HashEnum->CurrentBucket == nullptr)
    {
        // If the Index is less than the number of items in the hashTable and the key is null (meaning the main
        // node is empty), we increment the index to go to the next Main Node.
        while (HashEnum->Index < HashEnum->HashTable->NumBuckets &&
               HashEnum->HashTable->Buckets[HashEnum->Index].Key == nullptr)
        {
            HashEnum->Index++;
        }

        if (HashEnum->Index >= HashEnum->HashTable->NumBuckets)
        {
            return false;
        }

        // Go to the next Main Bucket.
        HashEnum->CurrentBucket = HashEnum->HashTable->Buckets + HashEnum->Index;
        HashEnum->Index++;
    }

    // Get the Key Value pair.
    *Key = HashEnum->CurrentBucket->Key;
    *Value = HashEnum->CurrentBucket->Value;

    // Set the current bucket to the next node in the chain of the main bucket.
    HashEnum->CurrentBucket = HashEnum->CurrentBucket->Next;

    return true;
}

static void
HashTableEnumDestroy(htable_enum *HashEnum)
{
    if (HashEnum == nullptr)
        return;
    free(HashEnum);
}


