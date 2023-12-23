#if !defined(VULKAN_BUFFER_H)

#include "defines.h"
#include "volk/volk.h"
#include "vulkan_renderer.h"
#include "vulkan_vertex_definitions.h"

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

void CreateVertexBuffers(shoora_vulkan_device *RenderDevice, shoora_vertex_info *Vertices, u32 VertexCount,
                         u32 *Indices, u32 IndexCount, shoora_vulkan_buffer *outVertexBuffer,
                         shoora_vulkan_buffer *outIndexBuffer);
void CreateVertexBuffers(shoora_vulkan_device *RenderDevice, shoora_model *Model,
                         shoora_vulkan_vertex_buffers *VertBuffers);

void DestroyVertexBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pVertexBuffer,
                         shoora_vulkan_buffer *pIndexBuffer);

void CreateUniformBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffers,
                          u32 UniformBufferCount, size_t Size);
shoora_vulkan_buffer CreateBuffer(shoora_vulkan_device *RenderDevice, VkBufferUsageFlags Usage,
                                                VkSharingMode SharingMode, VkMemoryPropertyFlags DesiredMemoryType,
                                                u8 *pData, size_t DataSize);

void DestroyUniformBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffer);
void DestroyUniformBuffers(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pUniformBuffers,
                           u32 UniformBufferCount);
void DestroyBuffer(shoora_vulkan_device *RenderDevice, shoora_vulkan_buffer *pBuffer);

#define VULKAN_BUFFER_H
#endif // VULKAN_BUFFER_H