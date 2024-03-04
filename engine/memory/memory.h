#if !defined(MEMORY_H)
#define MEMORY_H

#include <defines.h>
#include "freelist_allocator.h"

struct memory_arena
{
    void *Base;
    size_t Used = 0;
    size_t Size = 0;

    u32 StacksCount = 0;
};

struct stack_memory
{
    memory_arena *Arena;
    size_t ArenaUsedAtBegin;
};

void InitializeArena(memory_arena *Arena, size_t Size, void *BasePtr);
size_t GetArenaSizeRemaining(memory_arena *Arena, size_t Alignment = 4);
stack_memory BeginStackMemory(memory_arena *Arena);
void EndStackMemory(stack_memory TempMemory);
void ValidateArena(memory_arena *Arena);
void SubArena(memory_arena *Result, memory_arena *Arena, size_t Size, size_t Alignment = 16);

void *ShuAllocate_(memory_arena *Arena, size_t SizeInit, size_t Alignment = 4);
char *ShuAllocateString(memory_arena *Arena, const char *Source);

#define ShuAllocate(Arena, Size, ...) ShuAllocate_(Arena, Size, __VA_ARGS__)
#define ShuAllocateStruct(Arena, Type, ...) (Type *)ShuAllocate_(Arena, sizeof(Type), __VA_ARGS__)
#define ShuAllocateArray(Arena, Count, Type, ...) (Type *)ShuAllocate_(Arena, sizeof(Type)*Count, __VA_ARGS__)

#if _SHU_DEBUG
void memoryTest();
#endif

#endif // MEMORY_H