#if !defined(PLATFORM_H)
#include "defines.h"

#ifdef SHU_RENDERER_BACKEND_VULKAN
#include "volk/volk.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct shoora_platform_presentation_surface
{
#ifdef SHU_RENDERER_BACKEND_VULKAN
#ifdef VK_USE_PLATFORM_WIN32_KHR
    VkWin32SurfaceCreateInfoKHR *Win32SurfaceCreateInfo;
#endif
#endif
};

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
    LogType_DebugReportCallbackInfo,

    LogType_MaxCount
};

enum KeyState
{
    SHU_KEYSTATE_PRESS,
    SHU_KEYSTATE_DOWN,
    SHU_KEYSTATE_RELEASE,

    SHU_KEYSTATE_MAX_COUNT
};

#if defined(SHU_RENDERER_BACKEND_VULKAN) && defined(VK_USE_PLATFORM_WIN32_KHR)
typedef void func_window_resize(u32 Width, u32 Height);
#if 0
typedef void exit_application(const char *Reason);
typedef b8 check_keyboard_input_state(u8 KeyCode, KeyState State);
#endif
void FillVulkanWin32SurfaceCreateInfo(shoora_platform_presentation_surface *Surface);
#endif

struct platform_work_queue;
struct platform_memory
{
    void *PermMemory;
    size_t PermSize;
    void *FrameMemory;
    size_t FrameMemorySize;
};

struct shoora_app_info
{
    const char *AppName;

    u32 WindowWidth;
    u32 WindowHeight;

    func_window_resize *WindowResizeCallback;
    platform_work_queue *JobQueue;

    platform_memory GameMemory;
};

struct shoora_platform_frame_packet
{
    // b32 LeftMouseClicked;
    // b32 IsLeftMouseDown;
    f32 MouseXPos;
    f32 MouseYPos;

    f32 DeltaTime;
    u32 Fps;
};

struct platform_mutex
{
  private:
#if WIN32
    HANDLE MutexHandle;
#endif

  public:
    platform_mutex();
    ~platform_mutex();
    void Lock();
    void Unlock();
};

SHU_EXPORT void LogOutput(LogType LogType, const char *Format, ...);
SHU_EXPORT void LogInfo(const char *Format, ...);
SHU_EXPORT void LogDebug(const char *Format, ...);
SHU_EXPORT void LogWarn(const char *Format, ...);
SHU_EXPORT void LogError(const char *Format, ...);
SHU_EXPORT void LogFatal(const char *Format, ...);
SHU_EXPORT void LogTrace(const char *Format, ...);

SHU_EXPORT void LogUnformatted(const char *Message);
SHU_EXPORT void LogInfoUnformatted(const char *Message);
SHU_EXPORT void LogDebugUnformatted(const char *Message);
SHU_EXPORT void LogWarnUnformatted(const char *Message);
SHU_EXPORT void LogErrorUnformatted(const char *Message);
SHU_EXPORT void LogFatalUnformatted(const char *Message);
SHU_EXPORT void LogTraceUnformatted(const char *Message);

SHU_EXPORT void LogString(const char *String);

SHU_EXPORT void Platform_GenerateString(char *Buffer, u32 BufferSize, const char *Format, ...);
SHU_EXPORT void Platform_FreeMemory(void *Memory);

SHU_EXPORT void Platform_ExitApplication(const char *Reason);
SHU_EXPORT void Platform_Sleep(u32 ms);
SHU_EXPORT b8 Platform_GetKeyInputState(u8 KeyCode, KeyState State);
SHU_EXPORT void Platform_ToggleFPSCap();
SHU_EXPORT void Platform_SetFPS(i32 FPS);

#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Args)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

SHU_EXPORT void Platform_CompleteAllWork(platform_work_queue *Queue);
SHU_EXPORT void Platform_AddWorkEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);

#ifdef __cplusplus
}
#endif


#define PLATFORM_H
#endif // PLATFORM_H