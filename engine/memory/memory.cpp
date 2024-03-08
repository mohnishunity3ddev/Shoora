#include "memory.h"
#include "memory_utils.h"

static shoora_memory ShuMemory = {};

inline void InitializeTaskMemories(memory_arena *Arena);

inline memory_arena *
GetArena(shoora_memory_type Type)
{
    memory_arena *Result = nullptr;
    switch(Type)
    {
        case MEMTYPE_GLOBAL: { Result = &ShuMemory.PermanentArena; } break;
        case MEMTYPE_FRAME: { Result = &ShuMemory.FrameArena; } break;
        case MEMTYPE_FREELISTGLOBAL: { Result = &ShuMemory.GlobalFreelistArena; } break;
        case MEMTYPE_FREELISTFRAME: { Result = &ShuMemory.FrameFreelistArena; } break;

        SHU_INVALID_DEFAULT;
    }

    ASSERT(Result);
    return Result;
}

freelist_allocator *
GetFreelistAllocator(shoora_memory_type Type)
{
    freelist_allocator *Result = nullptr;

    if(Type == MEMTYPE_FREELISTGLOBAL) { Result = &ShuMemory.GlobalFreelistAllocator; }
    else if(Type == MEMTYPE_FREELISTFRAME) { Result = &ShuMemory.FrameFreelistAllocator; }

    ASSERT(Result != nullptr);
    return Result;
}

inline void *
ShuAllocate_(memory_arena *Arena, size_t SizeInit, size_t Alignment = 4)
{
    size_t Size = SizeInit;
    size_t AlignmentPadding = GetAlignmentPadding(Arena, Alignment);
    Size += AlignmentPadding;

    ASSERT(Size >= SizeInit);
    ASSERT((Arena->Used + Size) <= Arena->Size);

    void *Result = (void *)((u8 *)Arena->Base + (Arena->Used + AlignmentPadding));
    Arena->Used += Size;

    return Result;
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, size_t Size, size_t Alignment = 16)
{
    Result->Base = ShuAllocate_(Arena, Size, Alignment);
    Result->Used = 0;
    Result->Size = Size;
    Result->TempMemoryCount = 0;
}

inline void
InitializeArena(memory_arena *Arena, size_t Size, void *BasePtr)
{
    Arena->Base = (void *)((u8 *)BasePtr + GetAlignmentPadding(Arena, 16));
    Arena->Size = Size;
    Arena->Used = 0;
    Arena->TempMemoryCount = 0;
}

void
InitializeMemory(size_t GlobalMemSize, void *GlobalMem, size_t FrameMemSize, void *FrameMem)
{
    InitializeArena(&ShuMemory.PermanentArena, GlobalMemSize, GlobalMem);
    InitializeArena(&ShuMemory.FrameArena, FrameMemSize, FrameMem);

    InitializeTaskMemories(&ShuMemory.PermanentArena);

    size_t GlobalFreelistSize = MEGABYTES(128);
    SubArena(&ShuMemory.GlobalFreelistArena, &ShuMemory.PermanentArena, GlobalFreelistSize);
    void *GlobalFreelistMemory = ShuAllocate_(&ShuMemory.GlobalFreelistArena, GlobalFreelistSize);
    ShuMemory.GlobalFreelistAllocator.Initialize(GlobalFreelistMemory, GlobalFreelistSize);

    size_t FrameFreelistSize = MEGABYTES(64);
    SubArena(&ShuMemory.FrameFreelistArena, &ShuMemory.FrameArena, FrameFreelistSize);
    void *FrameFreelistMemory = ShuAllocate_(&ShuMemory.FrameFreelistArena, FrameFreelistSize);
    ShuMemory.FrameFreelistAllocator.Initialize(FrameFreelistMemory, FrameFreelistSize);
}

size_t
GetArenaSizeRemaining(memory_arena *Arena, size_t Alignment)
{
    ASSERT(Arena->Size > Arena->Used);

    size_t RemainingSize = Arena->Size - Arena->Used;
    return RemainingSize;
}

size_t
GetRemainingMemory(shoora_memory_type Type, size_t Alignment)
{
    memory_arena *Arena = GetArena(Type);

    size_t Result = GetArenaSizeRemaining(Arena, Alignment);

    ASSERT(Result > 0);
    return Result;
}

temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    ++Arena->TempMemoryCount;

    Result.Arena = Arena;
    Result.ArenaUsedAtBegin = Arena->Used;

    return Result;
}

void
EndTemporaryMemory(temporary_memory TempMemory)
{
    memory_arena *Arena = TempMemory.Arena;

    ASSERT(Arena->Used >= TempMemory.ArenaUsedAtBegin);
    Arena->Used = TempMemory.ArenaUsedAtBegin;

    ASSERT(Arena->TempMemoryCount > 0);
    --Arena->TempMemoryCount;
}

void
ValidateArena(memory_arena *Arena)
{
    ASSERT(Arena->TempMemoryCount == 0);
}

void
ValidateArena(shoora_memory_type Type)
{
    memory_arena *Arena = GetArena(Type);
    ValidateArena(Arena);
}


void *
ShuAllocate_(size_t SizeInit, shoora_memory_type Type, size_t Alignment)
{
    memory_arena *Arena = GetArena(Type);
    return ShuAllocate_(Arena, SizeInit, Alignment);
}

char *
ShuAllocateString(memory_arena *Arena, const char *Source)
{
    u32 Len = StringLength(Source) + 1;

    char *Dest = (char *)ShuAllocate_(Arena, Len);
    StringCopy(Source, Dest);

    return Dest;
}

char *
ShuAllocateString(const char *Source, shoora_memory_type Type)
{
    memory_arena *Arena = GetArena(Type);
    return ShuAllocateString(Arena, Source);
}

void
SubArena(memory_arena *Result, shoora_memory_type Type, size_t Size, size_t Alignment)
{
    memory_arena *Arena = GetArena(Type);
    SubArena(Result, Arena, Size, Alignment);
}

// NOTE: Initialize TASK_COUNT number of MemoryArenas which will be used by the individual threads in the
// threadPool presnet in the platform layer.
inline void
InitializeTaskMemories(memory_arena *Arena)
{
    for (i32 TaskIndex = 0; TaskIndex < MAX_TASK_MEMORY_COUNT; ++TaskIndex)
    {
        task_with_memory *Task = ShuMemory.TaskMemories + TaskIndex;
        Task->BeingUsed = false;
        Task->TemporaryMemory = {};
        SubArena(&Task->Arena, Arena, MEGABYTES(4));
    }
}

task_with_memory *
GetTaskMemory()
{
    task_with_memory *FreeTask = nullptr;
    for (i32 i = 0; i < MAX_TASK_MEMORY_COUNT; ++i)
    {
        task_with_memory *Task = ShuMemory.TaskMemories + i;
        if(!Task->BeingUsed)
        {
            FreeTask = Task;
            ASSERT(FreeTask->Arena.Used == 0);
            FreeTask->TemporaryMemory = BeginTemporaryMemory(&FreeTask->Arena);
            break;
        }
    }

    return FreeTask;
}

void
FreeTaskMemory(task_with_memory *Task)
{
    EndTemporaryMemory(Task->TemporaryMemory);
    ASSERT(Task->Arena.TempMemoryCount == 0);

    // NOTE: Doing this since this free gets called directly from the thread when its ending.
    CompletePastWritesBeforeFutureWrites;
    Task->BeingUsed = false;
}

#if 0
void
memoryTest()
{
    size_t MemSize = MEGABYTES(256);
    void *Memory = malloc(MemSize);

    memory_arena Arena;
    InitializeArena(&Arena, MemSize, Memory);

    memory_arena PermArena, TransientArena;

    SubArena(&PermArena, &Arena, MEGABYTES(120));
    SubArena(&TransientArena, &Arena, MEGABYTES(120));

    temporary_memory StackMemory = BeginTemporaryMemory(&PermArena);
    char *myName = ShuAllocateString(StackMemory.Arena, "mani");
    char *myName2 = ShuAllocateString(StackMemory.Arena, "Mohnish Sharma brother!");
    EndTemporaryMemory(StackMemory);

    i32 *NumsArray = ShuAllocateArray(i32, 22, MEMTYPE_GLOBAL);
    for (i32 i = 0; i < 22; ++i)
    {
        NumsArray[i] = i;
    }

    free(Memory);
}
#endif


