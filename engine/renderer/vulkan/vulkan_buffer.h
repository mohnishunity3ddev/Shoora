#if !defined(VULKAN_BUFFER_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"

struct shoora_buffer_transition_info
{
    VkBuffer Buffer;
    VkAccessFlags CurrentAccess;
    VkAccessFlags NewAccess;

    // NOTE: If we chose EXCLUSIVE SHARING MODE for the buffer during its creation.
    // NOTE: Set these two to VK_QUEUE_FAMILY_IGNORED if we do not want to transfer the ownership
    u32 CurrentQueueFamily;
    u32 NewQueueFamily;
};

#define VULKAN_BUFFER_H
#endif // VULKAN_BUFFER_H