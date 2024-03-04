#include "memory.h"

#include <platform/platform.h>
#include "memory_utils.h"

void
freelist_allocator::Initialize(void *Memory, size_t Size)
{
    ASSERT(Memory != nullptr && Size > 0 && "You have to pass in something valid here!");

    this->Memory = Memory;
    this->TotalSize = Size;

    flNode *newNode = (flNode *)Memory;
    newNode->next = nullptr;
    newNode->data.BlockSize = Size;

    // NewNode

    Freelist.Insert(nullptr, newNode);
    this->IsInitialized = true;
}

void
freelist_allocator::FirstFit(const size_t &Alignment, const size_t &RequiredSize, size_t &OutPadding,
                             flNode **OutFoundNode, flNode **OutPreviousNode)
{
    flNode *CurrentNode = this->Freelist.head;
    flNode *Prev = nullptr;
    size_t PaddingForAlignment = 0;

    while(CurrentNode != nullptr)
    {
        PaddingForAlignment = GetAlignmentPaddingWithRequirement((size_t)CurrentNode, Alignment,
                                                                 freelist_allocator::AllocationHeaderSize);
        size_t TotalSize = RequiredSize + PaddingForAlignment;
        if(CurrentNode->data.BlockSize >= TotalSize)
        {
            break;
        }

        Prev = CurrentNode;
        CurrentNode = CurrentNode->next;
    }

    OutPadding = PaddingForAlignment;
    *OutFoundNode = CurrentNode;
    *OutPreviousNode = Prev;
}

void *
freelist_allocator::Allocate(size_t RequiredSize, const size_t Alignment)
{
    ASSERT(this->IsInitialized);

    if (RequiredSize <= freelist_allocator::AllocationHeaderSize)
    {
        LogWarn("The header size is %zu and you are trying to allocate %zu bytes. This is not efficient since the "
                "header size required for a memory block is more or close to the size you want to allocate.\n",
                freelist_allocator::AllocationHeaderSize, RequiredSize);
    }

    // NOTE: This padding when added to the address of the free node gives an aligned address aligned to the
    // "Alignment" that was input here. This padding also includes the allocation header size.
    size_t HeaderSizePlusAlignPadding = 0;
    flNode *FreeNode = nullptr, *PreviousNode = nullptr;

    // NOTE: Find the first free block that fits our requirements.
    this->FirstFit(Alignment, RequiredSize, HeaderSizePlusAlignPadding, &FreeNode, &PreviousNode);
    ASSERT(FreeNode != nullptr);

    // NOTE: Wasteful padding space required just for memory alignment.
    size_t AlignmentPadding = HeaderSizePlusAlignPadding - freelist_allocator::AllocationHeaderSize;
    size_t AllocationBlockSize = RequiredSize + HeaderSizePlusAlignPadding;
    size_t RemainingSpace = FreeNode->data.BlockSize - AllocationBlockSize;

    ASSERT(RemainingSpace > 0 && "Freelist should have enough space for this allocation");

    // NOTE: Create a new free node, which has remaining space.
    flNode *NextFreeNode = (flNode *)((char *)FreeNode + AllocationBlockSize);
    NextFreeNode->data.BlockSize = RemainingSpace;
    this->Freelist.Insert(FreeNode, NextFreeNode);

    LogInfo("[Allocate]: Memory Address of free node: %zu.\n", (size_t)FreeNode);

    // NOTE: Remove the freenode that we found for the current allocation since its now being used for allocation.
    this->Freelist.Remove(PreviousNode, FreeNode);


    // NOTE: Create the data block
    // AlignmentPadding + allocation header size was padding which ensured alignment to the given boundary.
    // If I get the allocation header address at freenode + alignmentPadding, fill out the header. this will
    // advance the memory further by allocation header size. This way FreeNode + (AlignmentPadding +
    // headerSize)[padding] will lead to an aligned memory address. So our dataMemory that we return is aligned.
    auto *AllocationHeader = (freelist_allocator::allocation_header *)((char *)FreeNode + AlignmentPadding);
    AllocationHeader->AlignmentPadding = AlignmentPadding;
    AllocationHeader->BlockSize = AllocationBlockSize;

    // NOTE: This should be an aligned address.
    void *DataMemory = (void *)((char *)AllocationHeader + freelist_allocator::AllocationHeaderSize);

    // NOTE: Making sure the DataAddress is snapped to the alignment boundary
    ASSERT((((size_t)DataMemory % Alignment) == 0) && "Data address should be aligned!");
    return DataMemory;
}

void
freelist_allocator::Free(void *MemoryPtr)
{
    ASSERT(MemoryPtr != nullptr);

    // NOTE: The memory was always on the aligned boundary when allocation was done. Allocation Header contains
    // info on how many bytes were used to align the boundary. The Allocation header directly precedes this memory.

    // NOTE: If you look at the allocation function, this is how we were allocating memory.
    // FreeNode Address(which was removed) + AlignmentPadding + AllocationHeaderSize takes us to the Data memory.
    // And it should be this one! The allocation header contains the alignment information. Also the size of the
    // memory which was allocated. So we go this "MemoryPtr"(which is passed in here) - allocationHeaderSize -
    // AlignmentPadding. THis will be the address of the freeNode which we now have to add to the freelist to
    // deallocate memory.

    auto *AllocationHeader = (freelist_allocator::allocation_header *)((char *)MemoryPtr - freelist_allocator::AllocationHeaderSize);
    size_t AllocatedBlockSize = AllocationHeader->BlockSize;

    flNode *NewFreeNode = (flNode *)((char *)AllocationHeader - AllocationHeader->AlignmentPadding);
    size_t FreeBlockSize = AllocationHeader->BlockSize;
    NewFreeNode->data.BlockSize = FreeBlockSize;
    NewFreeNode->next = nullptr;

    // NOTE: We have the FreeNode that contains the allocation for the memory passed in here. Now we need to add it
    // to freelist for deallocation. We have to find the correct position where we want to add the node to the
    // freelist. correct position is such that the nodes are in increasing order of their address. this is to make
    // merging two contiguous free blocks into one.
    flNode *Curr = Freelist.head;
    flNode *Prev = nullptr;
    while(Curr != nullptr)
    {
        if(((size_t)NewFreeNode) < ((size_t)Curr))
        {
            Freelist.Insert(Prev, NewFreeNode);
            break;
        }

        Prev = Curr;
        Curr = Curr->next;
    }

    // NOTE: Now that we have added the freenode, now we want to check whether the added freenode is contiguous
    // with the next and/or previous freenode. If it does, then we merge all contiguous freeblocks into one big
    // combined freeblock.
    if (NewFreeNode->next != nullptr &&
        (size_t)((char *)NewFreeNode + NewFreeNode->data.BlockSize) == (size_t)NewFreeNode->next)
    {
        // NOTE: the freenode aligns perfectly with its next freenode. merging both current and next blocks into
        // one.
        NewFreeNode->data.BlockSize += NewFreeNode->next->data.BlockSize;
        Freelist.Remove(NewFreeNode, NewFreeNode->next);
    }

    if (Prev != nullptr &&
        (size_t)((char *)Prev + Prev->data.BlockSize) == (size_t)NewFreeNode)
    {
        // NOTE: the freenode aligns perfectly with its next freenode. merging both current and next blocks into
        // one.
        Prev->data.BlockSize += Prev->next->data.BlockSize;
        Freelist.Remove(Prev, NewFreeNode);
    }
}

#if _SHU_DEBUG

void
freelist_allocator_test()
{
    const size_t MemSize = 4*36;
    void *Mem = nullptr;
    size_t Alignment = 32;
    Mem = malloc(MemSize);

    ASSERT(Mem != nullptr);

    freelist_allocator Allocator(Mem, MemSize);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());

    int32 *i1 = (i32 *)Allocator.Allocate(sizeof(i32), Alignment);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    int32 *i2 = (i32 *)Allocator.Allocate(sizeof(i32), Alignment);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    int32 *i3 = (i32 *)Allocator.Allocate(sizeof(i32), Alignment);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    int32 *i4 = (i32 *)Allocator.Allocate(sizeof(i32), Alignment);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());

    *i1 = 1;
    *i2 = 3;
    *i3 = 3;
    *i4 = 7;

    ASSERT(Allocator.Freelist.numItems == 1);
    Allocator.Free(i4);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    ASSERT(Allocator.Freelist.numItems == 1);
    Allocator.Free(i1);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    ASSERT(Allocator.Freelist.numItems == 2);
    Allocator.Free(i3);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    ASSERT(Allocator.Freelist.numItems == 2);
    Allocator.Free(i2);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    ASSERT(Allocator.Freelist.numItems == 1);

    ASSERT(Allocator.DEBUGGetRemainingSpace() == MemSize);

    free(Mem);
}

#include <utils/random/random.h>
void
memory_utils_test()
{
    void *m = malloc(2048);

    shu::rand Rand;
    for (i32 i = 0; i < 1000; ++i)
    {
        size_t tMemAddress = Rand.NextUInt32();
        size_t tAlign = Rand.RangeBetweenUInt32(1, 2048);
        size_t tRequiredSize = Rand.RangeBetweenUInt32(1, 2048);
        size_t test = GetAlignmentPaddingWithRequirement(tMemAddress, tAlign, tRequiredSize);
    }

    size_t p1 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 16);
    size_t p2 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 32);
    size_t p3 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 64);
    size_t p4 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 128);
    size_t p5 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 256);
    size_t p6 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 512);

    p1 = GetAlignmentPaddingWithRequirement((size_t)m, 16, 512);
    p2 = GetAlignmentPaddingWithRequirement((size_t)m, 32, 512);
    p3 = GetAlignmentPaddingWithRequirement((size_t)m, 64, 512);
    p4 = GetAlignmentPaddingWithRequirement((size_t)m, 128, 512);
    p5 = GetAlignmentPaddingWithRequirement((size_t)m, 256, 512);
    p6 = GetAlignmentPaddingWithRequirement((size_t)m, 512, 512);
}

size_t
freelist_allocator::DEBUGGetRemainingSpace()
{
    size_t Result = 0;
    flNode *fNode = Freelist.head;

    while(fNode != nullptr)
    {
        Result += fNode->data.BlockSize;

        fNode = fNode->next;
    }

    return Result;
}
#endif

