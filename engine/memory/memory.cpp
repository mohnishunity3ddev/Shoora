#include "memory.h"
#include "memory_utils.h"

void
InitializeArena(memory_arena *Arena, size_t Size, void *BasePtr)
{
    Arena->Base = (void *)((u8 *)BasePtr + GetAlignmentPadding(Arena, 16));
    Arena->Size = Size;
    Arena->Used = 0;
    Arena->TempMemoryCount = 0;
}

size_t
GetArenaSizeRemaining(memory_arena *Arena, size_t Alignment)
{
    ASSERT(Arena->Size > Arena->Used);

    size_t RemainingSize = Arena->Size - Arena->Used;
    return RemainingSize;
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
SubArena(memory_arena *Result, memory_arena *Arena, size_t Size, size_t Alignment)
{
    Result->Base = ShuAllocate_(Arena, Size, Alignment);
    Result->Used = 0;
    Result->Size = Size;
    Result->TempMemoryCount = 0;
}

void *
ShuAllocate_(memory_arena *Arena, size_t SizeInit, size_t Alignment)
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



char *
ShuAllocateString(memory_arena *Arena, const char *Source)
{
    u32 Len = StringLength(Source) + 1;

    char *Dest = (char *)ShuAllocate_(Arena, Len);
    StringCopy(Source, Dest);

    return Dest;
}

static task_with_memory TaskMemories[MAX_TASK_MEMORY_COUNT];

void
InitializeTaskMemories(memory_arena *Arena)
{
    for (i32 TaskIndex = 0;
         TaskIndex < MAX_TASK_MEMORY_COUNT;
         ++TaskIndex)
    {
        task_with_memory *Task = TaskMemories + TaskIndex;
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
        task_with_memory *Task = TaskMemories + i;
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

#if _SHU_DEBUG
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

    i32 *NumsArray = ShuAllocateArray(&TransientArena, 22, i32);
    for (i32 i = 0; i < 22; ++i)
    {
        NumsArray[i] = i;
    }

    free(Memory);
}
#endif


