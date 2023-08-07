#if !defined(VULKAN_COMMAND_BUFFER_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"
#include "vulkan_device.h"

struct shura_command_buffer_allocate_info
{
    shura_queue_type QueueType;
    VkCommandBufferLevel Level;
    u32 BufferCount;
};

void AllocateCommandBuffers(shura_vulkan_context *Context, shura_command_buffer_allocate_info *AllocInfos,
                            u32 AllocInfoCount);

void BeginCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex,
                        VkCommandBufferUsageFlags Usage);
void EndCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex);

void BeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags Usage,
                        VkCommandBufferInheritanceInfo *pInheritanace, b32 *pIsRecording);
void EndCommandBuffer(VkCommandBuffer Buffer, b32 *pIsRecording);

#define VULKAN_COMMAND_BUFFER_H
#endif // VULKAN_COMMAND_BUFFER_H