#if !defined(MEMORY_UTILS_H)
#define MEMORY_UTILS_H

#include <defines.h>
#include <platform/platform.h>

size_t
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