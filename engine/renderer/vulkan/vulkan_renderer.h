#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "platform/platform.h"
#include "volk/volk.h"
#include "vulkan_defines.h"
#include "vulkan_device.h"
#include "math/math.h"

struct shoora_vulkan_debug
{
    VkDebugUtilsMessengerEXT Messenger;
    VkDebugReportCallbackEXT ReportCallback;
};

struct shoora_vulkan_queue
{
    // TODO: Shouldn't this be an array!
    VkQueue Handle;
    shoora_queue_type Type;
    u32 Count;
    u32 FamilyIndex;
    // TODO)): Make this Dynamic
    f32 Priorities[8] =
    {
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
};

struct shoora_vulkan_command_pool
{
    VkCommandPool Handle;
    shoora_queue_type Type;
    b32 IsTransient;
};

struct shoora_vulkan_command_buffer_handle
{
    VkCommandBuffer Handle;
    b32 IsRecording;
    VkCommandPool *CommandPool;
};

struct shoora_vulkan_device
{
    VkPhysicalDevice PhysicalDevice;
    VkPhysicalDeviceProperties DeviceProperties;
    VkPhysicalDeviceFeatures Features;
    VkPhysicalDeviceMemoryProperties MemoryProperties;

    VkDevice LogicalDevice;

    // TODO)): Make these Dynamic. Use Pointers instead.
    u32 PresentationQueueIndex;
    b32 IsGraphicsQueueForPresentation;

    u32 GraphicsQueueFamilyInternalIndex;
    u32 TransferQueueFamilyInternalIndex;
    u32 ComputeQueueFamilyInternalIndex;
    shoora_vulkan_queue QueueFamilies[SHU_VK_MAX_QUEUE_FAMILY_COUNT];
    // For short-lived commands.

    VkCommandPool GraphicsCommandPoolTransient;
    VkCommandPool GraphicsCommandPool;
    VkCommandPool TransferCommandPoolTransient;
    VkCommandPool TransferCommandPool;
    shoora_vulkan_command_pool TransientCommandPools[SHU_VK_MAX_QUEUE_FAMILY_COUNT];
    shoora_vulkan_command_pool CommandPools[SHU_VK_MAX_QUEUE_FAMILY_COUNT];

    u32 QueueFamilyCount;
};

struct shoora_vulkan_buffer
{
    VkBuffer Handle;
    VkDeviceMemory Memory;
    VkDeviceSize MemSize;
    void *pMapped = nullptr;
};

struct shoora_vulkan_swapchain
{
    VkSurfaceKHR Surface;
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;

    VkSurfaceFormatKHR SurfaceFormat;
    VkFormat DepthFormat;

    u32 CurrentImageIndex;
    VkExtent2D ImageDimensions;
    VkImageUsageFlags ImageUsageFlags;
    VkPresentModeKHR PresentMode;
    VkSurfaceTransformFlagBitsKHR TransformFlagBits;

    VkSwapchainKHR SwapchainHandle;

    // TODO)): Make this Dynamic!
    u32 ImageCount;
    VkImage Images[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    VkImageView ImageViews[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    VkFramebuffer ImageFramebuffers[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];

    VkDescriptorSetLayout UniformSetLayout;
    // TODO)): This pipeline layout needs to be removed from here!
    VkPipelineLayout UniformPipelineLayout;
    VkDescriptorPool UniformDescriptorPool;
    VkDescriptorSet UniformDescriptorSets[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    shoora_vulkan_buffer UniformBuffers[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    u32 UniformDataSize;

    shoora_vulkan_command_buffer_handle DrawCommandBuffers[SHU_MAX_FRAMES_IN_FLIGHT];
};

struct shoora_vulkan_semaphore_handle
{
    VkSemaphore Handle;
    b32 IsSignaled;
};

struct shoora_vulkan_fence_handle
{
    VkFence Handle;
    b32 IsSignaled;
};

struct shoora_vulkan_synchronization
{
    shoora_vulkan_semaphore_handle ImageAvailableSemaphores[SHU_MAX_FRAMES_IN_FLIGHT];
    shoora_vulkan_semaphore_handle RenderFinishedSemaphores[SHU_MAX_FRAMES_IN_FLIGHT];
    shoora_vulkan_fence_handle Fences[SHU_MAX_FRAMES_IN_FLIGHT];
};

struct shoora_vulkan_pipeline
{
    VkPipelineLayout GraphicsPipelineLayout;
    VkPipelineLayout WireframePipelineLayout;
    VkPipeline GraphicsPipeline;
    VkPipeline WireframeGraphicsPipeline;
};

struct shoora_vulkan_context
{
    VkInstance Instance;
    shoora_vulkan_debug Debug;
    shoora_vulkan_device Device;
    shoora_vulkan_swapchain Swapchain;
    VkRenderPass GraphicsRenderPass;
    shoora_vulkan_pipeline Pipeline;
    shoora_vulkan_buffer VertexBuffer;
    shoora_vulkan_buffer IndexBuffer;
    shoora_vulkan_synchronization SyncHandles;

    b32 IsInitialized;
    u32 CurrentFrame;
    u32 FrameCounter;
};

void InitializeVulkanRenderer(shoora_vulkan_context *VulkanContext, shoora_app_info *AppInfo);
void DrawFrameInVulkan();
void DestroyVulkanRenderer(shoora_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H