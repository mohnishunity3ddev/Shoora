#if !defined(MEMORY_UTILS_H)
#define MEMORY_UTILS_H

#include <defines.h>
#include <platform/platform.h>
#include "memory.h"

// NOTE: padding to add to the memory arena's Used, so that it is aligned to some alignment byte boundary that you
// want.
inline size_t
GetAlignmentPadding(memory_arena *Arena, size_t Alignment = 4)
{
    ASSERTPow2(Alignment);

    size_t MemoryPtr = (size_t)Arena->Base + Arena->Used;
    size_t AlignmentPadding = 0;
    size_t AlignmentMask = Alignment - 1;
    // If MemoryPtr is NOT aligned.
    if (MemoryPtr & AlignmentMask)
    {
        AlignmentPadding = Alignment - (MemoryPtr & AlignmentMask); // Align it.
    }

    return AlignmentPadding;
}

inline size_t
GetAlignmentPaddingWithRequirement(const size_t &RawNumber, const size_t &Alignment,
                                   const size_t &RequiredSize)
{
    size_t AlignedNum = AlignAsGeneric(RawNumber + RequiredSize, Alignment);
    size_t PaddingForAlignment = AlignedNum - RawNumber;

#if _SHU_DEBUG
    ASSERT(AlignedNum > RawNumber);
    ASSERT(PaddingForAlignment >= RequiredSize);
    if(Alignment != 0)
    {
        ASSERT((RawNumber + PaddingForAlignment) % Alignment == 0);
    }
    ASSERT((RawNumber + PaddingForAlignment) > RawNumber);

    // LogInfo("Address: 0x%zu, Alignment: %zu, RequiredSize: %zu, Padding: %zu, AlignedAddress: 0x%zu.\n", RawNumber,
    //         Alignment, RequiredSize, PaddingForAlignment, (RawNumber + PaddingForAlignment));
#endif

    return PaddingForAlignment;
}

#endif