#include "freelist_allocator.h"

#include "memory_utils.h"
#include <platform/platform.h>

void
freelist_allocator::Initialize(void *Memory, size_t Size)
{
    ASSERT(Memory != nullptr && Size > 0 && "You have to pass in something valid here!");

    this->Memory = Memory;
    this->TotalSize = Size;

    flNode *newNode = (flNode *)Memory;
    newNode->next = nullptr;
    newNode->data.BlockSize = Size;

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

    while (CurrentNode != nullptr)
    {
        PaddingForAlignment = GetAlignmentPaddingWithRequirement((size_t)CurrentNode, Alignment,
                                                                 freelist_allocator::AllocationHeaderSize);
        size_t TotalSize = RequiredSize + PaddingForAlignment;
        if (CurrentNode->data.BlockSize >= TotalSize)
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
    // ASSERT(FreeNode != nullptr);

    if(FreeNode != nullptr)
    {
        // NOTE: Wasteful padding space required just for memory alignment.
        size_t AlignmentPadding = HeaderSizePlusAlignPadding - freelist_allocator::AllocationHeaderSize;
        size_t AllocationBlockSize = RequiredSize + HeaderSizePlusAlignPadding;

        ASSERT(FreeNode->data.BlockSize >= AllocationBlockSize);
        size_t RemainingSpace = FreeNode->data.BlockSize - AllocationBlockSize;

        if(RemainingSpace == 0)
        {
            int x = 0;
        }
        ASSERT(RemainingSpace >= 0 && "Freelist should have enough space for this allocation");

        // NOTE: Create a new free node, which has remaining space.
        if(RemainingSpace > 0)
        {
            flNode *NextFreeNode = (flNode *)((u8 *)FreeNode + AllocationBlockSize);
            NextFreeNode->data.BlockSize = RemainingSpace;
            this->Freelist.Insert(FreeNode, NextFreeNode);
        }

        LogInfo("[Allocate]: Memory Address of free node: %zu.\n", (size_t)FreeNode);

        // NOTE: Remove the freenode that we found for the current allocation since its now being used for allocation.
        this->Freelist.Remove(PreviousNode, FreeNode);

        // NOTE: Create the data block
        // AlignmentPadding + allocation header size was padding which ensured alignment to the given boundary.
        // If I get the allocation header address at freenode + alignmentPadding, fill out the header. this will
        // advance the memory further by allocation header size. This way FreeNode + (AlignmentPadding +
        // headerSize)[padding] will lead to an aligned memory address. So our dataMemory that we return is aligned.
        auto *AllocationHeader = (freelist_allocation_header *)((u8 *)FreeNode + AlignmentPadding);
        AllocationHeader->AlignmentPadding = AlignmentPadding;
        AllocationHeader->BlockSize = AllocationBlockSize;

        // NOTE: This should be an aligned address.
        void *DataMemory = (void *)((u8 *)AllocationHeader + freelist_allocator::AllocationHeaderSize);

        // NOTE: Making sure the DataAddress is snapped to the alignment boundary
        // ASSERT((((size_t)DataMemory % Alignment) == 0) && "Data address should be aligned!");
        return DataMemory;
    }
    else
    {
        LogFatalUnformatted("There is no space available in the freelist!\n");
        return nullptr;
    }
}

void *
freelist_allocator::ReAllocate(void *MemoryPtr, size_t NewSize, const size_t Alignment)
{
    ASSERT(MemoryPtr != nullptr);

    auto *AllocationHeader = (freelist_allocation_header *)((u8 *)MemoryPtr -
                                                            freelist_allocator::AllocationHeaderSize);
    size_t HeaderPlusAlign = freelist_allocator::AllocationHeaderSize + AllocationHeader->AlignmentPadding;
    size_t OldSize = AllocationHeader->BlockSize - HeaderPlusAlign;

    return this->ReAllocateSized(MemoryPtr, OldSize, NewSize, Alignment);
}

void *
freelist_allocator::ReAllocateSized(void *OldMemoryPtr, size_t OldSize, size_t NewSize, const size_t Alignment)
{
    ASSERT(OldMemoryPtr != nullptr);

    // NOTE: The Allocation Header contains info about the allocated memory. It is always Header Size(16 bytes) behind the
    // allocated memory pointer.
    auto *AllocationHeader = (freelist_allocation_header *)((u8 *)OldMemoryPtr -
                                                            freelist_allocator::AllocationHeaderSize);
    void *NewMemoryPtr = OldMemoryPtr;
    if (OldSize < NewSize)
    {
        b32 FoundContiguousFreeblock = false;

        // NOTE: The size already allocated is less than the new size that we want here(So we need more memory than
        // already allocated). Look for a free block immediately after this which fits the requirements. If there
        // is, subtract the EXTRA MEMORY that we need from that free block and update its block size. If there is
        // no immediately succedding free block, in that case, we would have to look for a free block which takes
        // the whole New Size, and copy all the contents to that new block and then return. after copying we have
        // to allocate that new block and free this block and add it to the freelist.
        flNode *CurrFreeBlock = Freelist.head;
        flNode *PrevFreeBlock = nullptr;
        while(CurrFreeBlock != nullptr)
        {
            if((size_t)CurrFreeBlock > (size_t)OldMemoryPtr)
            {
                // NOTE: Check if a free block lines up with the old allocation
                if((u8 *)CurrFreeBlock == (u8 *)OldMemoryPtr + OldSize)
                {
                    size_t ExtraSpace = (NewSize - OldSize);
                    // NOTE: Check if the lined up free block has enough space to fit the new size.
                    if(CurrFreeBlock->data.BlockSize >= ExtraSpace)
                    {
                        AllocationHeader->BlockSize += ExtraSpace;

                        size_t FreeblockSpaceLeft = CurrFreeBlock->data.BlockSize - ExtraSpace;
                        // NOTE: Check if the current Freeblock gets wholly consumed by the ExtraSpace required for
                        // the new allocation.
                        if(FreeblockSpaceLeft == 0)
                        {
                            // NOTE: The Freeblock has been wholly consumed, hence, remove it from the freelist.
                            Freelist.Remove(PrevFreeBlock, CurrFreeBlock);
                        }
                        else
                        {
                            // NOTE: A part of the space left in freeblock has been used, so remove that much space
                            // from the free block.
                            CurrFreeBlock->data.BlockSize = FreeblockSpaceLeft;
                        }

                        FoundContiguousFreeblock = true;
                    }
                }

                // NOTE: The first Block which comes after the old allocation(CurrentFreeBlock > MemoryPtr) is the
                // ONLY candidate that we need to check for continuity. It is a given that the next free blocks
                // will in any case come after the old allocation since we have sorted the freelist to have
                // ascending order of memory addresses. so we need not go through the entire freelist looking for
                // continuity with the old allocation and just break here after we encountered first "AFTER"
                // freeblock.
                break;
            }

            PrevFreeBlock = CurrFreeBlock;
            CurrFreeBlock = CurrFreeBlock->next;
        }

        // NOTE: We could not a free block which lines up with the old allocation. In this case, look for an
        // entirely new free block, copy the contents of old to this new location and set that as the pointer to
        // memory you return.
        if(!FoundContiguousFreeblock)
        {
            // NOTE: Set the Memory that we return to the newly allocated memory containing old data.
            NewMemoryPtr = this->Allocate(NewSize, Alignment);

            if(NewMemoryPtr)
            {
                // NOTE: Copy the old data from the old memoryPtr to this new MemoryPtr.
                SHU_MEMCOPY(OldMemoryPtr, NewMemoryPtr, OldSize);

                // NOTE: Freeing the old memory allocation since we got a new one and we have copied all data from
                // old to new.
                this->Free(OldMemoryPtr);
            }
            else
            {
                LogErrorUnformatted("[ALLOCATOR: ReAlloc]Could not allocate for new size since the allocator has run "
                                    "out of space. Sucks to be you brother! :(.\n");
            }
        }
    }
    else if (OldSize > NewSize)
    {
        // NOTE: The New Size requires is less than the old size allocated. We calculate the difference and set
        // that as the free block.
        size_t ExtraSpaceToFree = OldSize - NewSize;

        // NOTE: Updating the old memory header to reflect the reduced size.
        AllocationHeader->BlockSize -= ExtraSpaceToFree;

        // NOTE: Calculating the address of the starting of a new free block chunk containing 'ExtraSpaceToFree'
        // bytes to be added to the freelist.
        flNode *NewFreeNode = (flNode *)((u8 *)OldMemoryPtr + ExtraSpaceToFree);
        NewFreeNode->data.BlockSize = ExtraSpaceToFree;

        // NOTE: Traverse the Linked list.
        b32 Inserted = false;
        flNode *CurrFreeNode = Freelist.head;
        flNode *PrevFreeNode = nullptr;
        while(CurrFreeNode != nullptr)
        {
            // NOTE: We have to enforce the freelist to contain free block chunks in ascending order of their
            // memory addresses. That's why the check here.
            if ((size_t)NewFreeNode < (size_t)CurrFreeNode)
            {
                Freelist.Insert(PrevFreeNode, NewFreeNode);
                Inserted = true;
                break;
            }

            PrevFreeNode = CurrFreeNode;
            CurrFreeNode = CurrFreeNode->next;
        }

        // NOTE: If the free block chunk's memory address is larger than all free nodes in the freelist. Add it at
        // the end as its tail.
        if(!Inserted)
        {
            Freelist.Insert(PrevFreeNode, NewFreeNode);
        }
    }

    ASSERT(NewMemoryPtr);
    return NewMemoryPtr;
}

void
freelist_allocator::Free(void *MemoryPtr)
{
    if(MemoryPtr == nullptr)
    {
        LogWarnUnformatted("[Allocator::Free]: Ptr passed in here was nullptr!\n");
        return;
    }

    // NOTE: The memory was always on the aligned boundary when allocation was done. Allocation Header contains
    // info on how many bytes were used to align the boundary. The Allocation header directly precedes this memory.

    // NOTE: If you look at the allocation function, this is how we were allocating memory.
    // FreeNode Address(which was removed) + AlignmentPadding + AllocationHeaderSize takes us to the Data memory.
    // And it should be this one! The allocation header contains the alignment information. Also the size of the
    // memory which was allocated. So we go this "MemoryPtr"(which is passed in here) - allocationHeaderSize -
    // AlignmentPadding. THis will be the address of the freeNode which we now have to add to the freelist to
    // deallocate memory.

    auto *AllocationHeader = (freelist_allocation_header *)((u8 *)MemoryPtr -
                                                            freelist_allocator::AllocationHeaderSize);
    size_t AllocatedBlockSize = AllocationHeader->BlockSize;

    flNode *NewFreeNode = (flNode *)((u8 *)AllocationHeader - AllocationHeader->AlignmentPadding);
    size_t FreeBlockSize = AllocationHeader->BlockSize;
    NewFreeNode->data.BlockSize = FreeBlockSize;
    NewFreeNode->next = nullptr;

    // NOTE: We have the FreeNode that contains the allocation for the memory passed in here. Now we need to add it
    // to freelist for deallocation. We have to find the correct position where we want to add the node to the
    // freelist. correct position is such that the nodes are in increasing order of their address. this is to make
    // merging two contiguous free blocks into one.
    flNode *Curr = Freelist.head;
    if(Curr == nullptr)
    {
        ASSERT(this->Freelist.numItems == 0 &&
               "If the head of the freelist is null, then numItems should be zero!");
        Freelist.Insert(nullptr, NewFreeNode);

        ASSERT(NewFreeNode->next == nullptr);
    }
    else
    {
        flNode *Prev = nullptr;
        b32 Inserted = false;
        while (Curr != nullptr)
        {
            // NOTE: Trying to insert in ascending order of freenode addresses.
            if (((size_t)NewFreeNode) < ((size_t)Curr))
            {
                Freelist.Insert(Prev, NewFreeNode);
                Inserted = true;
                break;
            }

            Prev = Curr;
            Curr = Curr->next;
        }

        if(!Inserted)
        {
            Freelist.Insert(Prev, NewFreeNode);
        }

        // NOTE: Now that we have added the freenode, now we want to check whether the added freenode is contiguous
        // with the next and/or previous freenode. If it does, then we merge all contiguous freeblocks into one big
        // combined freeblock.
        if (NewFreeNode->next != nullptr &&
            (size_t)((u8 *)NewFreeNode + NewFreeNode->data.BlockSize) == (size_t)NewFreeNode->next)
        {
            // NOTE: the freenode aligns perfectly with its next freenode. merging both current and next blocks into
            // one.
            NewFreeNode->data.BlockSize += NewFreeNode->next->data.BlockSize;
            Freelist.Remove(NewFreeNode, NewFreeNode->next);
        }

        if (Prev != nullptr && (size_t)((u8 *)Prev + Prev->data.BlockSize) == (size_t)NewFreeNode)
        {
            // NOTE: the freenode aligns perfectly with its next freenode. merging both current and next blocks into
            // one.
            Prev->data.BlockSize += Prev->next->data.BlockSize;
            Freelist.Remove(Prev, NewFreeNode);
        }
    }
}

#if _SHU_DEBUG

void
freelist_allocator_test()
{
    const size_t MemSize = 80;
    void *Mem = nullptr;
    size_t Alignment = 0;
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

    int32 *i5 = (i32 *)Allocator.Allocate(sizeof(i32), Alignment);
    ASSERT(i5 == nullptr);

    *i1 = 1;
    *i2 = 2;
    *i3 = 3;
    *i4 = 4;

    auto *i1Header = (freelist_allocation_header *)((u8 *)i1 - sizeof(freelist_allocation_header));
    auto *i2Header = (freelist_allocation_header *)((u8 *)i2 - sizeof(freelist_allocation_header));
    auto *i3Header = (freelist_allocation_header *)((u8 *)i3 - sizeof(freelist_allocation_header));
    auto *i4Header = (freelist_allocation_header *)((u8 *)i4 - sizeof(freelist_allocation_header));

    // NOTE: RE-Alloc Test.
    ASSERT(Allocator.Freelist.numItems == 0);
    Allocator.Free(i2);
    i2 = nullptr;
    ASSERT(Allocator.Freelist.numItems == 1);
    Allocator.Free(i1);
    i1 = nullptr;
    ASSERT(Allocator.Freelist.numItems == 1);
    size_t SizeRemaining = Allocator.DEBUGGetRemainingSpace();

    size_t allocationHeaderSize = 16;
    i32 *newi3 = (i32 *)Allocator.ReAllocate(i3, 2*sizeof(i32) + allocationHeaderSize, Alignment);
    ASSERT((size_t)newi3 == ((size_t)Mem + allocationHeaderSize));
    ASSERT(*i3 == 3);
    ASSERT(newi3[0] == *i3);
    i3 = nullptr;
    ASSERT(Allocator.Freelist.numItems == 1);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());

    i32 _t = 255;
    size_t CurrentFreeSize = Allocator.DEBUGGetRemainingSpace();
    i16 *newi4 = (i16 *)Allocator.ReAllocate(i4, sizeof(i16), Alignment);
    ASSERT(Allocator.Freelist.numItems == 2);
    i4 = nullptr;
    // NOTE: newi4[0] represents the i16 value already held in i4 which is 4.
    // NOTE: newi4[1] represents the i16 value of the new freenode block size which is 2. Why? because i4 was a i32
    // value(4 bytes), the newi4 is an i16 value (2 bytes). Since we reallocated 2 bytes from a 4 byte allocation
    // block, the new free block size will be 2 bytes. newi4[1] represents the free block address now.
    ASSERT(newi4[0] == 4 && newi4[1] == 2);
    size_t NewFreeSize = Allocator.DEBUGGetRemainingSpace();
    ASSERT(NewFreeSize == CurrentFreeSize + sizeof(i16));

    // NOTE: Free Test.
    ASSERT(Allocator.Freelist.numItems == 2);
    Allocator.Free(newi4);
    LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());

    if (i1 != nullptr)
    {
        ASSERT(Allocator.Freelist.numItems == 1);
        Allocator.Free(i1);
        LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
    }

    ASSERT(Allocator.Freelist.numItems == 1);
    if (i3 != nullptr)
    {
        auto *newi3Header = (freelist_allocation_header *)((u8 *)newi3 - sizeof(freelist_allocation_header));
        Allocator.Free(i3);
        LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
        ASSERT(Allocator.Freelist.numItems == 1);
    }

    if (i2 != nullptr)
    {
        Allocator.Free(i2);
        LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
        ASSERT(Allocator.Freelist.numItems == 1);
    }

    if(newi3 != nullptr)
    {
        Allocator.Free(newi3);
        LogDebug("Free space: %zu.\n", Allocator.DEBUGGetRemainingSpace());
        ASSERT(Allocator.Freelist.numItems == 1);
    }

    ASSERT(Allocator.DEBUGGetRemainingSpace() == MemSize);

    // NOTE: not doing this here, since the linked list destructor in the Freelist will call its destructor and do
    // this for us.
    // free(Mem);
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
