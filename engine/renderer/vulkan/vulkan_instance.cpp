#include "vulkan_instance.h"

// TODO)): Remove this!
#include <cstring>
#include "vulkan_renderer.h"
#include "platform/platform.h"
#include <memory>

b32
CheckAvailableInstanceExtensions(const char **ppRequiredInstanceExtensions, u32 RequiredInstanceExtensionCount)
{
    u32 AvailableExtensionCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &AvailableExtensionCount, 0));
    // VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &AvailableExtensionCount, 0));

    ASSERT((AvailableExtensionCount > 0) && (AvailableExtensionCount <= 32) &&
           (AvailableExtensionCount >= RequiredInstanceExtensionCount));

    // VkExtensionProperties AvailableExtensions[32];
    VkExtensionProperties *AvailableExtensions = (VkExtensionProperties *)malloc(AvailableExtensionCount *
                                                                                 sizeof(VkExtensionProperties));
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &AvailableExtensionCount, AvailableExtensions));

    i32 FoundExtensionCount = 0;
    for(i32 RequiredExtensionIndex = 0;
        RequiredExtensionIndex < RequiredInstanceExtensionCount;
        ++RequiredExtensionIndex)
    {
        const char *RequiredExtensionName = *(ppRequiredInstanceExtensions + RequiredExtensionIndex);

        for(i32 AvailableExtensionIndex = 0;
            AvailableExtensionIndex < AvailableExtensionCount;
            ++AvailableExtensionIndex)
        {
            VkExtensionProperties Extension = AvailableExtensions[AvailableExtensionIndex];

            // TODO)): String Equality Function
            if (!strcmp(Extension.extensionName, RequiredExtensionName))
            {
                ++FoundExtensionCount;
                break;
            }
        }
    }

    free(AvailableExtensions);
    b32 Result = (FoundExtensionCount == RequiredInstanceExtensionCount);
    return Result;
}

b32
CheckAvailableInstanceLayers(const char **ppRequiredLayers, u32 RequiredLayerCount)
{
    u32 AvailableLayerCount = 0;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&AvailableLayerCount, 0));

    ASSERT((AvailableLayerCount > 0) && (AvailableLayerCount <= 32) &&
           (AvailableLayerCount >= RequiredLayerCount));

    // VkLayerProperties AvailableLayers[32];
    VkLayerProperties *AvailableLayers = (VkLayerProperties *)malloc(AvailableLayerCount *
                                                                     sizeof(VkLayerProperties));
    VK_CHECK(vkEnumerateInstanceLayerProperties(&AvailableLayerCount, AvailableLayers));

    i32 FoundLayerCount = 0;
    for(i32 RequiredLayerIndex = 0;
        RequiredLayerIndex < RequiredLayerCount;
        ++RequiredLayerIndex)
    {

        const char *RequiredLayerName = *(ppRequiredLayers + RequiredLayerIndex);

        for(i32 AvailableLayerIndex = 0;
            AvailableLayerIndex < AvailableLayerCount;
            ++AvailableLayerIndex)
        {
            VkLayerProperties Layer = AvailableLayers[AvailableLayerIndex];

            // TODO)): String Equality Function
            if (!strcmp(Layer.layerName, RequiredLayerName))
            {
                ++FoundLayerCount;
                break;
            }
        }
    }

    free(AvailableLayers);
    b32 Result = (FoundLayerCount == RequiredLayerCount);
    return Result;
}

b32
CreateVulkanInstance(shoora_vulkan_context *VulkanContext, shoora_instance_create_info *ShuraInstanceCreateInfo)
{
    const char *AppName = ShuraInstanceCreateInfo->AppName;
    const char **ppRequiredInstanceExtensions = ShuraInstanceCreateInfo->ppRequiredInstanceExtensions;
    u32 RequiredInstanceExtensionCount = ShuraInstanceCreateInfo->RequiredInstanceExtensionCount;
    const char **ppRequiredInstanceLayers = ShuraInstanceCreateInfo->ppRequiredInstanceLayers;
    u32 RequiredInstanceLayerCount = ShuraInstanceCreateInfo->RequiredInstanceLayerCount;

    b32 RequiredLayersAvailable = CheckAvailableInstanceLayers(ppRequiredInstanceLayers,
                                                               RequiredInstanceLayerCount);
    if(!RequiredLayersAvailable)
    {
        LogFatal("Required Layers NOT Available for creating Vulkan Instance\n");
        return false;
    }

    b32 RequiredExtensionsAvailable = CheckAvailableInstanceExtensions(ppRequiredInstanceExtensions,
                                                                       RequiredInstanceExtensionCount);
    if(!RequiredExtensionsAvailable)
    {
        LogFatal("Required Extensions NOT Available for creating Vulkan Instance\n");
        return false;
    }

    VkApplicationInfo AppInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    AppInfo.pNext = 0;
    AppInfo.pApplicationName = AppName;
    AppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    AppInfo.pEngineName = SHU_ENGINE_NAME;
    AppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo InstanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    InstanceCreateInfo.pNext = 0;
    InstanceCreateInfo.flags = 0;
    InstanceCreateInfo.pApplicationInfo = &AppInfo;
    InstanceCreateInfo.enabledLayerCount = RequiredInstanceLayerCount;
    InstanceCreateInfo.ppEnabledLayerNames = RequiredInstanceLayerCount > 0 ? ppRequiredInstanceLayers : 0;
    InstanceCreateInfo.enabledExtensionCount = RequiredInstanceExtensionCount;
    InstanceCreateInfo.ppEnabledExtensionNames = RequiredInstanceExtensionCount > 0 ? ppRequiredInstanceExtensions
                                                                                    : 0;

    // TODO)): I keep passing 0 as the Allocator. Might want to integrate Vulkan Memory Allocator from AMD.
    VK_CHECK(vkCreateInstance(&InstanceCreateInfo, 0, &VulkanContext->Instance));

    LogInfo("Created Vulkan Instance.\n");
    return true;
}

void
DestroyVulkanInstance(shoora_vulkan_context *VulkanContext)
{
    vkDestroyInstance(VulkanContext->Instance, 0);
    LogWarn("Destroyed Vulkan Instance!\n");
}
