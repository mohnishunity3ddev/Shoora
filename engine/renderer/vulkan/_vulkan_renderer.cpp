
#include "_vulkan_renderer.h"
#include <cstring>

static const char *RequiredLayers[] = {SHU_VULKAN_VALIDATION_KHRONOS};

static const char *RequiredExtensions[] =
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

// Utils
b32
CheckAvailableInstanceExtensions()
{
    u32 AvailableExtensionCount = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &AvailableExtensionCount, 0));
    // VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &AvailableExtensionCount, 0));

    ASSERT((AvailableExtensionCount > 0) &&
           (AvailableExtensionCount <= 256));

    VkExtensionProperties AvailableExtensions[256];
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &AvailableExtensionCount, AvailableExtensions));

    i32 FoundExtensionCount = 0;
    for (i32 RequiredExtensionIndex = 0;
         RequiredExtensionIndex < ARRAY_SIZE(RequiredExtensions);
         ++RequiredExtensionIndex)
    {
        const char *RequiredExtensionName = RequiredExtensions[RequiredExtensionIndex];

        for(i32 AvailableExtensionIndex = 0;
            AvailableExtensionIndex < AvailableExtensionCount;
            ++AvailableExtensionIndex)
        {
            VkExtensionProperties Extension = AvailableExtensions[AvailableExtensionIndex];

            // TODO)): String Equality Function
            if(!strcmp(Extension.extensionName, RequiredExtensionName))
            {
                ++FoundExtensionCount;
                break;
            }
        }
    }

    b32 Result = (FoundExtensionCount == ARRAY_SIZE(RequiredExtensions));
    return Result;
}

void
CreateVulkanInstance(VkInstance *Instance, const char *AppName)
{
    VkApplicationInfo AppInfo = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    AppInfo.pNext = 0;
    AppInfo.pApplicationName = AppName;
    AppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    AppInfo.pEngineName = SHU_ENGINE_NAME;
    AppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    u32 RequiredExtensionCount = ARRAY_SIZE(RequiredExtensions);
    u32 RequiredLayerCount = ARRAY_SIZE(RequiredLayers);

    VkInstanceCreateInfo InstanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    InstanceCreateInfo.pNext = 0;
    InstanceCreateInfo.flags = 0;
    InstanceCreateInfo.pApplicationInfo = &AppInfo;
    InstanceCreateInfo.enabledLayerCount = RequiredLayerCount;
    InstanceCreateInfo.ppEnabledLayerNames = RequiredLayerCount > 0 ? RequiredLayers : 0;
    InstanceCreateInfo.enabledExtensionCount = RequiredExtensionCount;
    InstanceCreateInfo.ppEnabledExtensionNames = RequiredExtensionCount > 0 ? RequiredExtensions : 0;

    // TODO)): I keep passing 0 as the Allocator. Might want to integrate Vulkan Memory Allocator from AMD.
    VK_CHECK(vkCreateInstance(&InstanceCreateInfo, 0, Instance));
}

void
DestroyVulkanInstance(VkInstance Instance)
{
    vkDestroyInstance(Instance, 0);
}

void DestroyVulkanRenderer(vulkan_context *Context)
{
    DestroyVulkanInstance(Context->Instance);
}

void
InitializeVulkanRenderer(vulkan_context *VulkanContext, const char *AppName)
{
    volkInitialize();

    b32 RequiredExtensionsAvailable = CheckAvailableInstanceExtensions();
    ASSERT(RequiredExtensionsAvailable);

    CreateVulkanInstance(&VulkanContext->Instance, AppName);
    volkLoadInstance(VulkanContext->Instance);
}
