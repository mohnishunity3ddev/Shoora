#if !defined(VULKAN_COMMAND_BUFFER_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"
#include "vulkan_device.h"

struct shoora_command_buffer_allocate_info
{
    shoora_queue_type QueueType;
    VkCommandBufferLevel Level;
    u32 BufferCount;
};

void AllocateCommandBuffers(shoora_vulkan_context *Context, shoora_command_buffer_allocate_info *AllocInfos,
                            u32 AllocInfoCount);

void BeginCommandBuffer(shoora_vulkan_command_buffer *Buffer, u32 InternalBufferIndex,
                        VkCommandBufferUsageFlags Usage);
void BeginCommandBuffer(shoora_vulkan_command_buffer_handle *CmdBuffer, VkCommandBufferUsageFlags Usage,
                        VkCommandBufferInheritanceInfo *pInheritanace);

void EndCommandBuffer(shoora_vulkan_command_buffer *Buffer, u32 InternalBufferIndex);
void EndCommandBuffer(shoora_vulkan_command_buffer_handle *CmdBuffer);

void ResetCommandBuffer(shoora_vulkan_command_buffer_handle *CmdBufferHandle, b32 ReleaseResources);
void ResetCommandBuffer(shoora_vulkan_command_buffer *Buffer, u32 InternalBufferIndex, b32 ReleaseResources);

shoora_vulkan_command_buffer *GetCommandBufferGroupForQueue(shoora_vulkan_context *Context, shoora_queue_type Type);

#define VULKAN_COMMAND_BUFFER_H
#endif // VULKAN_COMMAND_BUFFER_H