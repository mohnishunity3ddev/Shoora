#if !defined(MEMORY_H)
#define MEMORY_H

#include <defines.h>
#include "freelist_allocator.h"

struct memory_arena
{
    void *Base;
    size_t Used = 0;
    size_t Size = 0;

    u32 TempMemoryCount = 0;
};

struct temporary_memory
{
    memory_arena *Arena;
    size_t ArenaUsedAtBegin;
};

void InitializeArena(memory_arena *Arena, size_t Size, void *BasePtr);
size_t GetArenaSizeRemaining(memory_arena *Arena, size_t Alignment = 4);
temporary_memory BeginTemporaryMemory(memory_arena *Arena);
void EndTemporaryMemory(temporary_memory TempMemory);
void ValidateArena(memory_arena *Arena);
void SubArena(memory_arena *Result, memory_arena *Arena, size_t Size, size_t Alignment = 16);

void *ShuAllocate_(memory_arena *Arena, size_t SizeInit, size_t Alignment = 4);
char *ShuAllocateString(memory_arena *Arena, const char *Source);

#define ShuAllocate(Arena, Size, ...) ShuAllocate_(Arena, Size, __VA_ARGS__)
#define ShuAllocateStruct(Arena, Type, ...) (Type *)ShuAllocate_(Arena, sizeof(Type), __VA_ARGS__)
#define ShuAllocateArray(Arena, Count, Type, ...) (Type *)ShuAllocate_(Arena, sizeof(Type)*Count, __VA_ARGS__)

#define MAX_TASK_MEMORY_COUNT 4
struct task_with_memory
{
    b32 BeingUsed;
    memory_arena Arena;
    // NOTE: This is used to reset the Arena after a thread is done with its work.
    temporary_memory TemporaryMemory;
};

// NOTE: Initialize TASK_COUNT number of MemoryArenas which will be used by the individual threads in the
// threadPool presnet in the platform layer.
void InitializeTaskMemories(memory_arena *Arena);

// NOTE: Gets called before starting a thread
task_with_memory *GetTaskMemory();
// NOTE: Gets called from a thread at its end.
void FreeTaskMemory(task_with_memory *Task);

#if _SHU_DEBUG
void memoryTest();
#endif

#endif // MEMORY_H