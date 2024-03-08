#if !defined(FREELIST_ALLOCATOR_H)
#define FREELIST_ALLOCATOR_H

#include <containers/linked_list/linked_list.h>

struct free_block_header
{
    size_t BlockSize;
};

struct freelist_allocation_header
{
    size_t AlignmentPadding;
    size_t BlockSize;
};

typedef singly_linked_list_node<free_block_header> flNode;
struct freelist_allocator
{
    freelist_allocator() : Memory(nullptr), TotalSize(0) {}
    explicit freelist_allocator(void *Mem, size_t Size) { Initialize(Mem, Size); }

    void Initialize(void *Memory, size_t Size);
    void *Allocate(size_t Size, const size_t Alignment = 4);
    // NOTE: Same as C's realloc
    void *ReAllocate(void *MemoryPtr, size_t NewSize, const size_t Alignment = 4);
    void *ReAllocateSized(void *MemoryPtr, size_t OldSize, size_t NewSize, const size_t Alignment = 4);
    void Free(void *Memory);

    const size_t AllocationHeaderSize = sizeof(freelist_allocation_header);
    const size_t FreeNodeHeaderSize = sizeof(free_block_header) + sizeof(void *);

  public:
    void *Memory = nullptr;
    size_t TotalSize = 0;

  private:
#if _SHU_DEBUG
  public:
    size_t DEBUGGetRemainingSpace();
    singly_linked_list<free_block_header> Freelist;

  private:
#else
    singly_linked_list<free_block_header> Freelist;
#endif

    void FirstFit(const size_t &Alignment, const size_t &RequiredSize, size_t &Padding, flNode **FoundNode,
                  flNode **PreviousNode);
    b32 IsInitialized = false;
};

#if _SHU_DEBUG
void freelist_allocator_test();
void dynamic_array_test();
void memory_utils_test();
#endif

#endif // FREELIST_ALLOCATOR_H