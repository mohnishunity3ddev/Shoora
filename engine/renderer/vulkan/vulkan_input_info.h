#if !defined(VULKAN_INPUT_INFO_H)
#include <volk/volk.h>

#include "defines.h"
#include "vulkan_command_buffer.h"
#include "vulkan_debug.h"
#include "vulkan_device.h"
#include "vulkan_instance.h"
#include "vulkan_renderer.h"
#include "vulkan_swapchain.h"
#include "vulkan_command_buffer.h"
#include "vulkan_synchronization.h"

const char *RequiredInstanceLayers[] =
{
#if _DEBUG
    SHU_VK_VALIDATION_KHRONOS
#endif
};

const char *RequiredInstanceExtensions[] =
{
    SHU_VK_EXTENSION_SURFACE,

#if defined(WIN32)
    SHU_VK_EXTENSION_WIN32_SURFACE,
#endif
#if defined(__APPLE__)
    #SHU_VULKAN_EXTENSION_MACOS_SURFACE,
#endif
#if defined(__linux__)
    SHU_VULKAN_EXTENSION_XCB_SURFACE,
#endif

#if defined(_DEBUG)
    SHU_VK_EXTENSION_DEBUG_UTILS,
    SHU_VK_EXTENSION_DEBUG_REPORT,
#endif
};

const char *RequiredDeviceExtensions[] =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

shoora_vulkan_debug_create_info DebugCreateInfo = {.SeverityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
                                                  .MessageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                                      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                                      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                                                  .ReportFlags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                                 VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                                 VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                                                 VK_DEBUG_REPORT_DEBUG_BIT_EXT};

shoora_instance_create_info ShuraInstanceCreateInfo = {.ppRequiredInstanceExtensions = RequiredInstanceExtensions,
                                                      .RequiredInstanceExtensionCount = ARRAY_SIZE(RequiredInstanceExtensions),
                                                      .ppRequiredInstanceLayers = RequiredInstanceLayers,
                                                      .RequiredInstanceLayerCount = ARRAY_SIZE(RequiredInstanceLayers)};

shoora_queue_info QueueInfos[] =
{
    {.Type = QueueType_Graphics, .QueueCount = 1},
    {.Type = QueueType_Compute, .QueueCount = 1},
    {.Type = QueueType_Transfer, .QueueCount = 1},
};
VkPhysicalDeviceFeatures DesiredFeatures =
{
    .geometryShader = VK_TRUE,
    .samplerAnisotropy = VK_TRUE
};
// Different Threads should have their own Command Pools.
shoora_command_pool_create_info CommandPoolCreateInfos[] =
{
    {
        .CreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .QueueType = QueueType_Graphics
    },
    {
        .CreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .QueueType = QueueType_Compute
    },
    {
        .CreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .QueueType = QueueType_Transfer
    }
};
shoora_device_create_info DeviceCreateInfo =
{
    .ppRequiredExtensions = RequiredDeviceExtensions,
    .RequiredExtensionCount = ARRAY_SIZE(RequiredDeviceExtensions),
    .DesiredFeatures = &DesiredFeatures,
    .pQueueCreateInfos = QueueInfos,
    .QueueCreateInfoCount = ARRAY_SIZE(QueueInfos),
    .pCommandPoolCreateInfos = CommandPoolCreateInfos,
    .CommandPoolCount = ARRAY_SIZE(CommandPoolCreateInfos)
};

// Swapchain
shoora_vulkan_swapchain_create_info SwapchainInfo =
{
    .DesiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR,
    .DesiredImageUsages = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .DesiredTransformFlagBits = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,

    .DesiredImageFormat = VK_FORMAT_B8G8R8A8_UNORM,
    .DesiredImageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
};

VkCommandPoolCreateFlags CommandPoolFlags = 0;
shoora_command_buffer_allocate_info Shu_BufferAllocInfos[] = {
    {
        .QueueType = QueueType_Graphics,
        .Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .BufferCount = 4
    },
    {
        .QueueType = QueueType_Compute,
        .Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .BufferCount = 3
    },
    {
        .QueueType = QueueType_Transfer,
        .Level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .BufferCount = 6
    }
};

// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT:
// If the command buffer will be submitted only once and then reset or
// re-recorded
//
// VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT
// If it is the secondary command buffer and is considered to be entirely
// inside a render pass
//
// VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
// If the command buffer needs to be resubmitted to a queue while it is
// still being executed on a device (before the previous submission of this
// command buffer has ended).

#define VULKAN_INPUT_INFO_H
#endif