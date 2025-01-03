#if !defined(PLATFORM_H)
#include "defines.h"

#ifdef SHU_RENDERER_BACKEND_VULKAN
#include "volk/volk.h"
#endif

#ifdef __cplusplus
extern "C" {
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

typedef void func_window_resize(u32 Width, u32 Height);
void Platform_GetWindowDetails(void **WindowHandle, void **WindowInstance);
void Platform_GetWindowSize(i32 *Width, i32 *Height);
b8 Platform_IsWindowReady();
#if 0
typedef void exit_application(const char *Reason);
typedef b8 check_keyboard_input_state(u8 KeyCode, KeyState State);
#endif

struct platform_work_queue;
struct platform_memory
{
    void *PermMemory;
    size_t PermSize;
    void *FrameMemory;
    size_t FrameMemorySize;
};

struct shoora_platform_app_info
{
    const char *AppName;

    /*
    u32 WindowWidth;
    u32 WindowHeight;
    func_window_resize *WindowResizeCallback;
    */

    platform_work_queue *JobQueue;

    platform_memory GameMemory;
};

/* this gets filled by the platform layer */
struct shoora_platform_frame_packet
{
    // b32 LeftMouseClicked;
    // b32 IsLeftMouseDown;
    f32 MouseXPos;
    f32 MouseYPos;

    f32 DeltaTime;
    u32 Fps;
};

#if 0
struct platform_mutex
{
  private:
#if _WIN32
    HANDLE MutexHandle;
#endif

  public:
    platform_mutex();
    ~platform_mutex();
    void Lock();
    void Unlock();
};
#endif

struct platform_read_file_result
{
    char *Path;
    u32 Size;
    u8 *Data;
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

SHU_EXPORT i32 Platform_GenerateString(char *Buffer, u32 BufferSize, const char *Format, ...);
SHU_EXPORT void Platform_FreeMemory(void *Memory);

SHU_EXPORT u32 Platform_GetRandomSeed();

SHU_EXPORT platform_read_file_result Platform_ReadFile(const char *Path);
SHU_EXPORT void Platform_FreeFileMemory(platform_read_file_result *File);
SHU_EXPORT b32 Platform_WriteFile(char *Filename, u32 Size, void *Data);

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