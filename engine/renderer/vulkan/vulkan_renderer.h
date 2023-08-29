#if !defined(_VULKAN_RENDERER_H)
#include "defines.h"
#include "math/math.h"
#include "platform/platform.h"
#include "volk/volk.h"
#include "vulkan_defines.h"
#include "vulkan_device.h"
#include "camera/camera.h"
#include <imgui.h>


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
    VkQueue GraphicsQueue, TransferQueue, ComputeQueue;
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

struct shoora_vulkan_image
{
    VkImage Handle;
    VkImageView ImageView;
    VkDeviceMemory ImageMemory;
};

struct shoora_vulkan_image_sampler
{
    shoora_vulkan_image Image;
    VkSampler Sampler;
};

struct shoora_vulkan_descriptor_resource
{

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

    VkSwapchainKHR Handle;

    // TODO)): Make this Dynamic!
    u32 ImageCount;
    VkImage Images[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    VkImageView ImageViews[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    shoora_vulkan_image DepthStencilImage;
    VkFramebuffer ImageFramebuffers[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];

    VkDescriptorPool UniformDescriptorPool;

    VkDescriptorSetLayout UniformSetLayout;
    VkDescriptorSet UniformDescriptorSets[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];
    shoora_vulkan_buffer UniformBuffers[SHU_VK_MAX_SWAPCHAIN_IMAGE_COUNT];

    VkDescriptorSetLayout SampledImageDescriptorSetLayout;
    VkDescriptorSet SampledImageDescriptorSet;
    shoora_vulkan_image_sampler UniformCombinedImageSampler;

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
    VkPipeline GraphicsPipeline;
    VkPipelineLayout GraphicsPipelineLayout;
    VkPipeline WireframeGraphicsPipeline;
    VkPipelineLayout WireframePipelineLayout;
};

struct shoora_imgui_push_constant_block
{
    Shu::vec2f Scale;
    Shu::vec2f Translate;
};

struct shoora_vulkan_imgui
{
    Shu::vec2u WindowDim;
    shoora_vulkan_device *RenderDevice;
    ImGuiStyle UIStyle;

    VkImage FontImage = VK_NULL_HANDLE;
    VkSampler FontSampler = VK_NULL_HANDLE;
    VkDeviceMemory FontMemory = VK_NULL_HANDLE;
    VkImageView FontImageView = VK_NULL_HANDLE;

    VkDescriptorPool DescriptorPool;
    VkDescriptorSetLayout DescriptorSetLayout;
    VkDescriptorSet DescriptorSet;

    VkPipelineLayout PipelineLayout;
    VkPipeline Pipeline;

    shoora_vulkan_buffer VertexBuffer;
    u32 VertexCount;
    shoora_vulkan_buffer IndexBuffer;
    u32 IndexCount;

    shoora_imgui_push_constant_block PushConstantBlock;
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

    shoora_camera Camera;

    shoora_vulkan_imgui ImContext;


    b32 IsInitialized;
    u32 CurrentFrame;
    u32 FrameCounter;
};

void InitializeVulkanRenderer(shoora_vulkan_context *VulkanContext, shoora_app_info *AppInfo);
void DrawFrameInVulkan(shoora_platform_frame_packet *FramePacket);
void DestroyVulkanRenderer(shoora_vulkan_context *Context);

#define _VULKAN_RENDERER_H
#endif // _VULKAN_RENDERER_H