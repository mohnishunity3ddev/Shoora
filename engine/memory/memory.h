#if !defined(MEMORY_H)
#define MEMORY_H

#include <defines.h>
#include "freelist_allocator.h"

#define MAX_TASK_MEMORY_COUNT 4

#define ShuAllocate(Size, MemType, ...) ShuAllocate_(Size, MemType, __VA_ARGS__)
#define ShuAllocateStruct(Type, MemType, ...) (Type *)ShuAllocate_(sizeof(Type), MemType, __VA_ARGS__)
#define ShuAllocateArray(Type, Count, MemType, ...) (Type *)ShuAllocate_(sizeof(Type)*Count, MemType, __VA_ARGS__)

#if 0
#define ShuAllocateGlobal(Size, ...) ShuAllocate_(Size, __VA_ARGS__)
#define ShuAllocateStructGlobal(Type, ...) (Type *)ShuAllocate_(sizeof(Type), __VA_ARGS__)
#define ShuAllocateArrayGlobal(Type, Count, ...) (Type *)ShuAllocate_(sizeof(Type)*Count, __VA_ARGS__)
#endif

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

struct task_with_memory
{
    b32 BeingUsed;
    memory_arena Arena;
    // NOTE: This is used to reset the Arena after a thread is done with its work.
    temporary_memory TemporaryMemory;
};

struct shoora_memory
{
    memory_arena PermanentArena;
    memory_arena FrameArena;

    memory_arena GlobalFreelistArena;
    memory_arena FrameFreelistArena;
    freelist_allocator GlobalFreelistAllocator;
    freelist_allocator FrameFreelistAllocator;

    task_with_memory TaskMemories[MAX_TASK_MEMORY_COUNT];
};

enum shoora_memory_type
{
    MEMTYPE_NONE,
    MEMTYPE_GLOBAL,
    MEMTYPE_FREELISTGLOBAL,
    MEMTYPE_FREELISTFRAME,
    MEMTYPE_FRAME,
};

memory_arena *GetArena(shoora_memory_type Type);

void InitializeMemory(size_t GlobalMemSize, void *GlobalMem, size_t FrameMemSize, void *FrameMem);
size_t GetRemainingMemory(shoora_memory_type Type, size_t Alignment = 4);
freelist_allocator *GetFreelistAllocator(shoora_memory_type Type);

temporary_memory BeginTemporaryMemory(memory_arena *Arena);
void EndTemporaryMemory(temporary_memory TempMemory);

void ValidateArena(shoora_memory_type Type);

void SubArena(memory_arena *Result, shoora_memory_type Type, size_t Size, size_t Alignment = 16);

void *ShuAllocate_(size_t SizeInit, shoora_memory_type Type = MEMTYPE_GLOBAL, size_t Alignment = 4);
char *ShuAllocateString(const char *Source, shoora_memory_type Type = MEMTYPE_GLOBAL);

// NOTE: Gets called before starting a thread
task_with_memory *GetTaskMemory();
// NOTE: Gets called from a thread at its end.
void FreeTaskMemory(task_with_memory *Task);

#if _SHU_DEBUG
void memoryTest();
#endif

#endif // MEMORY_H