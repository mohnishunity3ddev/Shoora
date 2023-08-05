#include "vulkan_debug.h"
#include "platform/platform.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverityFlags,
                    VkDebugUtilsMessageTypeFlagsEXT MessageTypeFlags,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                    void *pUserData)
{
    LogOutput("Validation Layer Message: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugReportCallback(VkDebugReportFlagsEXT Flags, VkDebugReportObjectTypeEXT ObjectType, u64 Object,
                          size_t Location, i32 MessageCode, const char *pLayerPrefix, const char *pMessage,
                          void *UserData)
{
    // if(Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    //     return VK_FALSE;
    LogOutput("Debug Callback(%s): %s\n", pLayerPrefix, pMessage);
    return VK_FALSE;
}

b32
SetupDebugCallbacks(shura_vulkan_context *Context, shura_vulkan_debug_create_info DebugCreateInfo)
{
    VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {};
    MessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    MessengerCreateInfo.messageSeverity = DebugCreateInfo.SeverityFlags;
    MessengerCreateInfo.messageType = DebugCreateInfo.MessageTypeFlags;
    MessengerCreateInfo.pfnUserCallback = &VulkanDebugCallback;
    MessengerCreateInfo.pUserData = 0;

    VK_CHECK(vkCreateDebugUtilsMessengerEXT(Context->Instance, &MessengerCreateInfo, 0,
                                            &Context->Debug.Messenger));

    VkDebugReportCallbackCreateInfoEXT ReportCallbackInfo = {};
    ReportCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ReportCallbackInfo.pNext = 0;
    ReportCallbackInfo.flags = DebugCreateInfo.ReportFlags;
    ReportCallbackInfo.pfnCallback = &VulkanDebugReportCallback;
    ReportCallbackInfo.pUserData = 0;

    VK_CHECK(vkCreateDebugReportCallbackEXT(Context->Instance, &ReportCallbackInfo, 0,
                                            &Context->Debug.ReportCallback));

    return true;
}

b32
SetVkObjectName(shura_vulkan_context *Context, void *Object, VkObjectType ObjectType, const char *Name)
{
    VkDebugUtilsObjectNameInfoEXT NameInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    NameInfo.pNext = 0;
    NameInfo.objectType = ObjectType;
    NameInfo.objectHandle = (u64)Object;
    NameInfo.pObjectName = Name;

    VK_CHECK(vkSetDebugUtilsObjectNameEXT(Context->LogicalDevice, &NameInfo));

    return true;
}

void
DestroyDebugUtilHandles(shura_vulkan_context *Context)
{
    vkDestroyDebugUtilsMessengerEXT(Context->Instance, Context->Debug.Messenger, 0);
    vkDestroyDebugReportCallbackEXT(Context->Instance, Context->Debug.ReportCallback, 0);

    LogOutput("Destroyed Debug Utils!\n");
}
