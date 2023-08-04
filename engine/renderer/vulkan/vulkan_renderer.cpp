
#include "vulkan_renderer.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "platform/platform.h"

const char *RequiredInstanceLayers[] =
{
#if _DEBUG
    SHU_VULKAN_VALIDATION_KHRONOS
#endif
};

const char *RequiredInstanceExtensions[] =
{
    SHU_VULKAN_EXTENSION_SURFACE,

#if defined(WIN32)
    SHU_VULKAN_EXTENSION_WIN32_SURFACE,
#endif
#if defined(__APPLE__)
    #SHU_VULKAN_EXTENSION_MACOS_SURFACE,
#endif
#if defined(__linux__)
    SHU_VULKAN_EXTENSION_XCB_SURFACE,
#endif

#if defined(_DEBUG)
    SHU_VULKAN_EXTENSION_DEBUG_UTILS,
    SHU_VULKAN_EXTENSION_DEBUG_REPORT,
#endif
};

const char *RequiredDeviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

void
InitializeVulkanRenderer(shura_vulkan_context *VulkanContext, const char *AppName)
{
    volkInitialize();

    // Get Vulkan instance
    shura_instance_create_info ShuraInstanceCreateInfo =
    {
        .AppName = AppName,
        .ppRequiredInstanceExtensions = RequiredInstanceExtensions,
        .RequiredInstanceExtensionCount = ARRAY_SIZE(RequiredInstanceExtensions),
        .ppRequiredInstanceLayers = RequiredInstanceLayers,
        .RequiredInstanceLayerCount = ARRAY_SIZE(RequiredInstanceLayers)
    };
    ASSERT(CreateVulkanInstance(VulkanContext, &ShuraInstanceCreateInfo));
    volkLoadInstance(VulkanContext->Instance);

    // Load Vulkan Logical device and Device Queues which will be used for rendering.
    shura_queue_info QueueInfos[] =
    {
        {.Type = QueueType_Graphics, .QueueCount = 2},
        {.Type = QueueType_Compute, .QueueCount = 2},
        {.Type = QueueType_Transfer, .QueueCount = 2},
    };

    VkPhysicalDeviceFeatures DesiredFeatures = {};
    DesiredFeatures.samplerAnisotropy = VK_TRUE;
    DesiredFeatures.geometryShader = VK_TRUE;

    shura_device_create_info DeviceCreateInfo =
    {
        .ppRequiredExtensions = RequiredDeviceExtensions,
        .RequiredExtensionCount = ARRAY_SIZE(RequiredDeviceExtensions),
        .DesiredFeatures = &DesiredFeatures,
        .pQueueCreateInfos = QueueInfos,
        .QueueCreateInfoCount = ARRAY_SIZE(QueueInfos)
    };
    CreateLogicalDeviceAndGetQueues(VulkanContext, &DeviceCreateInfo);
    volkLoadDevice(VulkanContext->LogicalDevice);
}

void
DestroyVulkanRenderer(shura_vulkan_context *Context)
{
    DestroyLogicalDevice(Context);
    DestroyVulkanInstance(Context);
    LogOutput("Destroyed Vulkan Renderer!\n");
}
