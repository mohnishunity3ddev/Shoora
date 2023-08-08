#include "vulkan_debug.h"
#include "platform/platform.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverityFlags,
                    VkDebugUtilsMessageTypeFlagsEXT MessageTypeFlags,
                    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                    void *pUserData)
{
    switch(MessageSeverityFlags)
    {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        {
            LogOutput(LogType_ValidationLayerInfo, "Validation Layer[VERBOSE]: %s\n", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        {
            LogOutput(LogType_ValidationLayerInfo, "Validation Layer[INFO]: %s\n", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        {
            LogOutput(LogType_Warn, "Validation Layer[WARN]: %s\n", pCallbackData->pMessage);
        } break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        {
            LogOutput(LogType_Error, "Validation Layer[ERROR]: %s\n", pCallbackData->pMessage);
        } break;
        default: {} break;
    }

    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugReportCallback(VkDebugReportFlagsEXT Flags, VkDebugReportObjectTypeEXT ObjectType, u64 Object,
                          size_t Location, i32 MessageCode, const char *pLayerPrefix, const char *pMessage,
                          void *UserData)
{
    // if(Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    //     return VK_FALSE;
    // TODO)): Read more about how to use this.
    // VK_DEBUG_REPORT_INFORMATION_BIT_EXT
    // VK_DEBUG_REPORT_WARNING_BIT_EXT
    // VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
    // VK_DEBUG_REPORT_ERROR_BIT_EXT
    // VK_DEBUG_REPORT_DEBUG_BIT_EXT
    // VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT
    if(Flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        LogOutput(LogType_DebugReportCallbackInfo, "DebugReport(%s)[INFO]: %s\n", pLayerPrefix, pMessage);
    }
    if(Flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        LogOutput(LogType_DebugReportCallbackInfo, "DebugReport(%s)[DEBUG]: %s\n", pLayerPrefix, pMessage);
    }
    if(Flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        LogOutput(LogType_Warn, "DebugReport(%s)[PERFORMANCE]: %s\n", pLayerPrefix, pMessage);
    }
    if(Flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        LogOutput(LogType_Warn, "DebugReport(%s)[WARN]: %s\n", pLayerPrefix, pMessage);
    }
    if (Flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        LogOutput(LogType_Error, "DebugReport(%s)[ERROR]: %s\n", pLayerPrefix, pMessage);
    }

    return VK_FALSE;
}

b32
SetupDebugCallbacks(shoora_vulkan_context *Context, shoora_vulkan_debug_create_info DebugCreateInfo)
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

    LogOutput(LogType_Info, "Vulkan Debug Utils have been created!\n");
    return true;
}

b32
SetVkObjectName(shoora_vulkan_context *Context, void *Object, VkObjectType ObjectType, const char *Name)
{
    VkDebugUtilsObjectNameInfoEXT NameInfo = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    NameInfo.pNext = 0;
    NameInfo.objectType = ObjectType;
    NameInfo.objectHandle = (u64)Object;
    NameInfo.pObjectName = Name;

    VkResult Result = vkSetDebugUtilsObjectNameEXT(Context->Device.LogicalDevice, &NameInfo);

    return true;
}

void
DestroyDebugUtilHandles(shoora_vulkan_context *Context)
{
    vkDestroyDebugUtilsMessengerEXT(Context->Instance, Context->Debug.Messenger, 0);
    vkDestroyDebugReportCallbackEXT(Context->Instance, Context->Debug.ReportCallback, 0);

    LogOutput(LogType_Info, "Destroyed Debug Utils!\n");
}
