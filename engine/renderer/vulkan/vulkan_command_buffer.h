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
void BeginCommandBuffer(shura_vulkan_command_buffer_handle *CmdBuffer, VkCommandBufferUsageFlags Usage,
                        VkCommandBufferInheritanceInfo *pInheritanace);

void EndCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex);
void EndCommandBuffer(shura_vulkan_command_buffer_handle *CmdBuffer);

void ResetCommandBuffer(shura_vulkan_command_buffer_handle *CmdBufferHandle, b32 ReleaseResources);
void ResetCommandBuffer(shura_vulkan_command_buffer *Buffer, u32 InternalBufferIndex, b32 ReleaseResources);

shura_vulkan_command_buffer *GetCommandBufferGroupForQueue(shura_vulkan_context *Context, shura_queue_type Type);

#define VULKAN_COMMAND_BUFFER_H
#endif // VULKAN_COMMAND_BUFFER_H