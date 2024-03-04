#if !defined(MEMORY_H)

#include <defines.h>
#include <containers/linked_list/linked_list.h>

enum allocator_type
{
    FREE_LIST_ALLOCATOR,
    STACK_ALLOCATOR,
    LINEAR_ALLOCATOR,
};

struct memory_arena
{
    void *Memory;
    size_t TotalSize;
};

#define flNode singly_linked_list_node<freelist_allocator::free_block_header>
struct freelist_allocator
{
    explicit freelist_allocator() : Memory(nullptr), TotalSize(0) {}
    explicit freelist_allocator(void *Mem, size_t Size) { Initialize(Mem, Size); }

    void Initialize(void *Memory, size_t Size);
    void *Allocate(size_t Size, const size_t Alignment = 8);
    void Free(void *Memory);

    struct free_block_header
    {
        size_t BlockSize;
    };

    struct allocation_header
    {
        size_t AlignmentPadding;
        size_t BlockSize;
    };
    const size_t AllocationHeaderSize = sizeof(allocation_header);

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
    void *Memory = nullptr;
    size_t TotalSize = 0;
    b32 IsInitialized = false;
};

#if _SHU_DEBUG
void freelist_allocator_test();
void memory_utils_test();
#endif

#define MEMORY_H
#endif // MEMORY_H