#if !defined(PLATFORM_H)
#include "defines.h"

#ifdef SHU_RENDERER_BACKEND_VULKAN
#include "volk/volk.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct shura_app_info
{
    const char *AppName;
};

struct shura_platform_presentation_surface
{
#ifdef SHU_RENDERER_BACKEND_VULKAN
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR *Win32SurfaceCreateInfo;
#endif
#endif
};

#if defined(SHU_RENDERER_BACKEND_VULKAN) && defined(VK_USE_PLATFORM_WIN32_KHR)
void FillVulkanWin32SurfaceCreateInfo(shura_platform_presentation_surface *Surface);
#endif

enum LogType
{
    // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
    LogType_Fatal,
    LogType_Error,
    LogType_Warn,
    LogType_Info,
    LogType_Debug,
    LogType_Trace,
    LogType_ValidationLayerInfo,

    LogType_MaxCount
};

SHU_EXPORT void LogOutput(LogType LogType, const char *Format, ...);

#ifdef __cplusplus
}
#endif


#define PLATFORM_H
#endif // PLATFORM_H