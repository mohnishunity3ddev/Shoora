#ifndef UNICODE
#define UNICODE
// #include <winuser.h>
// #include <winuser.h>
#endif

#include "defines.h"
#include "platform/platform.h"
#include "platform/windows/win_platform.h"
#include <Windows.h>
#include <shellapi.h> /* external debug console */

// TODO: I don't like this. Maybe move the main entry point to game and use the platform layer as a helper class.
#include <renderer/renderer_frontend.h>

// TODO)): Remove this and implement your own!
#include <stdio.h>

#define FPS_CAPPING_ENABLED 0
#define KEY_PRESS_MASK (1ULL << 15)

#if FPS_CAPPING_ENABLED
// TODO)): Get a different strategy for waiting times. TimeBeginPeriod decreases system performance as per spec.
#include <timeapi.h>
#endif

struct win32_window_context
{
    HWND Handle;
    HINSTANCE hInstance;
    HANDLE ConsoleHandle;
    // TODO: This is unused now since clearing will be done by vulkan. keeping it here for mainting alignment.
    HBRUSH ClearColor;
};

struct win32_state
{
    WINDOWPLACEMENT WindowPosition;
    u32 MonitorRefreshRate;
    void *MemoryBlock;
    size_t MemoryBlockSize;
    win32_window_context WindowContext;
    struct platform_input_state *InputState;
    int64 PerfFrequency;
    i32 FPS;
#if SHU_CRASH_DUMP_ENABLE
    FILE *WriteFp = nullptr;
#endif
    b16 isInitialized;
    b8 Running;
    b8 IsFPSCap;
    b8 IsResizing;
};

static win32_state Win32State;

static shoora_platform_app_info AppInfo =
{
    .AppName = "Placeholder App Name",
};

b8
Platform_IsWindowReady()
{
    return !Win32State.IsResizing;
}

RECT
Win32GetWindowRect(HWND WindowHandle)
{
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    return ClientRect;
}

void
Platform_GetWindowSize(i32 *Width, i32 *Height)
{
    RECT ScreenRect = Win32GetWindowRect(Win32State.WindowContext.Handle);
    i32 w = ScreenRect.right - ScreenRect.left;
    i32 h = ScreenRect.bottom - ScreenRect.top;
    ASSERT(w > 0 && h > 0);

    *Width = w;
    *Height = h;
}

void
Win32ToggleFullscreen(HWND Window)
{
    // NOTE: This follows Raymond Chens prescription for fullscreen toggling.
    // see : https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW) {
        // NON-FullScreen mode. Caching the windowPosition before going fullcreen. When we toggle back to normal
        // NON-Fullscreen mode in the else block, we will place the window using this cached windowPosition.
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if (GetWindowPlacement(Window, &Win32State.WindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP, MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &Win32State.WindowPosition);
        SetWindowPos(Window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

struct platform_input_button_state
{
    i32 ButtonTransitionsPerFrame;
    b8 IsCurrentlyDown;
    b8 IsReleased;
};

enum platform_input_mouse_button
{
    MouseButton_Left,
    MouseButton_Right,
    MouseButton_Middle,
    MouseButton_Extended0,
    MouseButton_Extended1,

    MouseButton_Count,
};

// TODO: All support for all keyboard keys. These are just a subset. Arrange them in sequence as given in
// TODO: platform::VirtualKeyCodes.
struct platform_input_state
{
    platform_input_button_state Buttons[SU_KEYCODE_MAX];
    f32 MouseXPos, MouseYPos;
};

LRESULT CALLBACK
Win32WindowCallback(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_QUIT:
        case WM_CLOSE:
        case WM_DESTROY: { Win32State.Running = false; } break;

        /*
        // This is useful when we want to know if the user is resizing the window manually using drag.
        case WM_ENTERSIZEMOVE: { Win32State.IsResizing = true; } break;
        case WM_SIZE: {
            if (Win32State.IsResizing) {
                AppInfo.WindowWidth = LOWORD(LParam);
                AppInfo.WindowHeight = HIWORD(LParam);
                // AppInfo.WindowResizeCallback(AppInfo.WindowWidth, AppInfo.WindowHeight);
            }
        } break;
        case WM_EXITSIZEMOVE: {
            if (Win32State.IsResizing) {
                Win32State.IsResizing = false;
            }
        } break;
        */

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            ASSERT(!"Keyboard input came in here through a non-dispatch message!");
            // NOTE: Moved the actual keyboard handling code down in the main WinMain loop.
        }
        break;

        // TODO: Check since Vulkan is taking over drawing, I don't need to handle this event on my own
        /*
        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            HDC DC = BeginPaint(WindowHandle, &PaintStruct);

            RECT ClientRect = Win32GetWindowRect(WindowHandle);
            FillRect(DC, &ClientRect, GlobalWin32WindowContext.ClearColor);

            ReleaseDC(WindowHandle, DC);
            EndPaint(WindowHandle, &PaintStruct);
        } break;
        */

        default: { Result = DefWindowProc(WindowHandle, Message, WParam, LParam); }
    }

    return Result;
}

void
Win32UpdateInputButtonState(platform_input_button_state *InputButtonState, b32 IsCurrentlyDown)
{
    if(InputButtonState->IsCurrentlyDown != IsCurrentlyDown) {
        ++InputButtonState->ButtonTransitionsPerFrame;
        InputButtonState->IsCurrentlyDown = IsCurrentlyDown;
        InputButtonState->IsReleased = !IsCurrentlyDown;
    }
}

b32
Win32InputKeyPressed(platform_input_button_state *InputState)
{
    b32 Result = ((InputState->ButtonTransitionsPerFrame > 1) ||
                  (InputState->ButtonTransitionsPerFrame == 1 && InputState->IsCurrentlyDown));
    return Result;
}

b32
Win32InputKeyReleased()
{
    // TODO)) To Implement
    return false;
}

void
Platform_ToggleFPSCap()
{
    Win32State.IsFPSCap = !Win32State.IsFPSCap;
}

void
Platform_SetFPS(i32 FPS)
{
    ASSERT(FPS > 0);
    Win32State.FPS = FPS;
}

// TODO: Perf Issue here. Instead of all these if-else checks, pass in an index here and directly check against the
// input array data.
b8
Platform_GetKeyInputState(u8 KeyCode, KeyState State)
{
    b8 Result = false;
    switch(State)
    {
        case SHU_KEYSTATE_PRESS: { return Win32InputKeyPressed(&Win32State.InputState->Buttons[KeyCode]); } break;
        case SHU_KEYSTATE_DOWN: { return Win32State.InputState->Buttons[KeyCode].IsCurrentlyDown; } break;
        case SHU_KEYSTATE_RELEASE: { return Win32State.InputState->Buttons[KeyCode].IsReleased; } break;
        default: { LogWarn("KeyState (%u) for Key(%u) was not identified!", (u32)State, (u32)KeyCode); } break;
    }

    return Result;
}

void
Win32LogLastError()
{
    DWORD ErrorCode = GetLastError();
    LPSTR ErrorMessage = nullptr;

    DWORD Result = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                  nullptr, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  (LPSTR)(&ErrorMessage), 0, nullptr);

    if(Result != 0) {
        char Buffer[512];
        sprintf_s(Buffer, ARRAY_SIZE(Buffer), "Error(%d): %s", ErrorCode, ErrorMessage);
        OutputDebugStringA(Buffer);
        LocalFree(ErrorMessage);
    }
    else
    {
        OutputDebugStringA("Could not get error code\n");
    }
}

void
Win32SetConsoleHandle()
{
    HANDLE Console = Win32State.WindowContext.ConsoleHandle;

    if(Console == 0) {
        Console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (Console == 0) {
            // If GetStdHandle fails, allocate a new console
            AllocConsole();
            Console = GetStdHandle(STD_OUTPUT_HANDLE);
            if (Console == 0) {
                // If GetStdHandle still fails after allocation, log the error and return
                Win32LogLastError();
                return;
            } else {
                Win32State.WindowContext.ConsoleHandle = Console;
            }
        } else {
            Win32State.WindowContext.ConsoleHandle = Console;
        }
    }
}

void
Win32PauseConsoleWindow()
{
    LogOutput(LogType_Info, "Press Enter/Escape to continue...\n");
    Sleep(100);

    while (true)
    {
        if (GetAsyncKeyState(VK_RETURN) & KEY_PRESS_MASK) {
            break;
        } else if (GetAsyncKeyState(VK_ESCAPE) & KEY_PRESS_MASK) {
            break;
        }

        Sleep(100);
    }
}


void
OutputDebugStringColor(LogType LogType, const char *message)
{
    static COLORREF Colors[] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(242, 188, 47),
                                RGB(40, 130, 255), RGB(128, 128, 128), RGB(96, 96, 96), RGB(255, 255, 0)};
    COLORREF Color = Colors[4];
    switch(LogType)
    {
        case LogType_DebugReportCallbackInfo:   { Color = Colors[6]; } break;
        case LogType_Trace:
        case LogType_ValidationLayerInfo:       { Color = Colors[5]; } break;
        case LogType_Fatal:
        case LogType_Error:                     { Color = Colors[0]; } break;
        case LogType_Warn:                      { Color = Colors[ARRAY_SIZE(Colors) - 1]; } break;
        case LogType_Info:                      { Color = Colors[1]; } break;
        case LogType_Debug:                     { Color = RGB(0, 255, 255); } break;
        default: { } break;
    }

    char buffer[8192];
    sprintf_s(buffer, sizeof(buffer), "\x1B[38;2;%d;%d;%dm%s\x1B[0m", GetRValue(Color),
              GetGValue(Color), GetBValue(Color), message);
    OutputDebugStringA(buffer);
}

void
OutputToConsole(LogType LogType, const char *Message)
{
#if SHU_CRASH_DUMP_ENABLE
    // TODO: Make this thread-safe.
    if (WriteFp == nullptr)
    {
        WriteFp = fopen("app_dump.txt", "a");
        if (WriteFp != nullptr)
        {
            fprintf(WriteFp, "%s", Message);
            fclose(WriteFp);
            WriteFp = nullptr;
        }
    }
#endif

    HANDLE Console = Win32State.WindowContext.ConsoleHandle;

    static u8 Levels[] = {64, 4, 6, 2, 1, 8, 8, 8};
    SetConsoleTextAttribute(Console, Levels[LogType]);

    OutputDebugStringColor(LogType, Message);

    u64 Length = strlen(Message);
    LPDWORD NumberWritten = 0;
    WriteConsoleA(Console, Message, (DWORD)Length, NumberWritten, 0);
}

// NOTE: EXPORTED FUNCTIONS
// TODO)): Move this into a platform independent class called Logger
void
LogOutput(LogType LogType, const char *Format, ...)
{
    char Buffer[4096];
    i32 Length = 0;

    if(Format)
    {
        va_list VarArgs;
        va_start(VarArgs, Format);
        Length = vsnprintf(Buffer, ARRAY_SIZE(Buffer), Format, VarArgs);
        va_end(VarArgs);
    }

    if((Length > 0) &&
       ((size_t)Length < ARRAY_SIZE(Buffer)))
    {
        // OutputDebugStringA(Buffer);
        OutputToConsole(LogType, Buffer);
    }
}

i32
Platform_GenerateString(char *Buffer, u32 BufferSize, const char *Format, ...)
{
    va_list VarArgs;
    va_start(VarArgs, Format);

    i32 len = vsnprintf(Buffer, BufferSize, Format, VarArgs);

    va_end(VarArgs);
    return len;
}

void
Log_(LogType Type, const char *Format, va_list VarArgs)
{
    char Buffer[4096];

    i32 Length = vsnprintf(Buffer, ARRAYSIZE(Buffer), Format, VarArgs);

    if(Length > 0 && Length < ARRAYSIZE(Buffer))
    {
        OutputToConsole(Type, Buffer);
    }
}

#define LOG_FORMAT(Type, Format)                                                                                  \
    va_list VarArgs;                                                                                              \
    va_start(VarArgs, Format);                                                                                    \
    Log_(Type, Format, VarArgs);                                                                                  \
    va_end(VarArgs);
void LogInfo(const char *Format, ...) { LOG_FORMAT(LogType_Info, Format); }
void LogDebug(const char *Format, ...) { LOG_FORMAT(LogType_Debug, Format); }
void LogWarn(const char *Format, ...) { LOG_FORMAT(LogType_Warn, Format); }
void LogError(const char *Format, ...) { LOG_FORMAT(LogType_Error, Format); }
void LogFatal(const char *Format, ...) { LOG_FORMAT(LogType_Fatal, Format); }
void LogTrace(const char *Format, ...) { LOG_FORMAT(LogType_Trace, Format); }

void LogUnformatted(const char *Message) { OutputDebugStringA(Message); }
void LogInfoUnformatted(const char *Message) { OutputToConsole(LogType_Info, Message); }
void LogDebugUnformatted(const char *Message) { OutputToConsole(LogType_Debug, Message); }
void LogWarnUnformatted(const char *Message) { OutputToConsole(LogType_Warn, Message); }
void LogErrorUnformatted(const char *Message) { OutputToConsole(LogType_Error, Message); }
void LogFatalUnformatted(const char *Message) { OutputToConsole(LogType_Fatal, Message); }
void LogTraceUnformatted(const char *Message) { OutputToConsole(LogType_Trace, Message); }

void
Platform_ExitApplication(const char *Reason)
{
    LogOutput(LogType_Fatal, Reason);

    Win32State.Running = false;
}

void
LogString(const char *String)
{
    OutputDebugStringA(String);
}

b32 MouseTracking = false;
void
Win32ProcessWindowsMessageQueue(HWND WindowHandle, platform_input_state *Input)
{
    MSG Message;
    while(PeekMessage(&Message, WindowHandle, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 KeyCode = Message.wParam;

                b8 KeyWasPreviouslyDown = ((Message.lParam & (1ULL << 30)) != 0);
                b8 KeyIsCurrentlyDown = ((Message.lParam & (1ULL << 31)) == 0);
                b8 KeyIsPressed = (KeyIsCurrentlyDown) &
                                  (KeyIsCurrentlyDown != KeyWasPreviouslyDown);

                if (KeyCode == SU_ESCAPE && KeyIsPressed) {
                    Win32State.Running = false;
                    return;
                }

                // LogInfo("Updating input state for %d\n", KeyCode);
                if (KeyCode == SU_SHIFT) {
                    b32 ShiftKeyDown = (GetKeyState(SU_LEFTSHIFT) & KEY_PRESS_MASK) == KEY_PRESS_MASK;
                    if (ShiftKeyDown) {
                        Win32UpdateInputButtonState(&Input->Buttons[SU_LEFTSHIFT], ShiftKeyDown);
                    }
                } else {
                    b8 IsAltKeyDown = (GetKeyState(SU_LEFTALT) & KEY_PRESS_MASK) == KEY_PRESS_MASK;
                    if (IsAltKeyDown) {
                        Win32UpdateInputButtonState(&Input->Buttons[SU_LEFTALT], KeyIsCurrentlyDown);
                        if (KeyCode == SU_F4 && KeyIsCurrentlyDown) {
                            Win32State.Running = false;
                            return;
                        } else if (KeyCode == SU_RETURN && KeyIsCurrentlyDown) {
                            Win32ToggleFullscreen(Win32State.WindowContext.Handle);
                            Win32UpdateInputButtonState(&Input->Buttons[SU_RETURN], KeyIsCurrentlyDown);
                        }
                    } else {
                        // NOTE: Update the rest of the buttons
                        Win32UpdateInputButtonState(&Input->Buttons[KeyCode], KeyIsCurrentlyDown);
                    }
                }

            } break;

            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            } break;
        }
    }
}

b32
ShouldCreateConsole(PWSTR pCmdLine)
{
    i32 Argc;
    LPWSTR *Argv = CommandLineToArgvW(pCmdLine, &Argc);

    if (Argv)
    {
        for(i32 Index = 0; Index < Argc; ++Index)
        {
                wchar_t *Arg = Argv[Index];
                if (wcscmp(Arg, L"vscode") == 0)
                {
                    return false;
                }
        }
        LocalFree(Argv);
    }

    return true;
}

inline LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)Win32State.PerfFrequency);
    return Result;
}

void
Win32SetOutOfBoundsCursor(HWND Handle)
{
    RECT Rect;
    GetWindowRect(Handle, &Rect);

    POINT MousePos;
    GetCursorPos(&MousePos);

    if(MousePos.x >= (Rect.right - 5))
    {
        MousePos.x = Rect.left + 5;
    }
    else if(MousePos.x < (Rect.left - 5))
    {
        MousePos.x = Rect.right - 5;
    }

    if(MousePos.y >= (Rect.bottom - 5))
    {
        MousePos.y = Rect.top + 5;
    }
    else if(MousePos.y < (Rect.top - 5))
    {
        MousePos.y = Rect.bottom - 5;
    }

    SetCursorPos(MousePos.x, MousePos.y);
}

#if GET_STACK_LIMITS
void
Win32GetStackLimits()
{
    PULONG stackLimit{};
    PULONG stackBase{};
    GetCurrentThreadStackLimits((PULONG_PTR)&stackLimit, (PULONG_PTR)&stackBase);
    // Calculate the available stack space
    SIZE_T stackSpace, sL = (SIZE_T)stackLimit, sB = (SIZE_T)stackBase;
    if(sL > sB) { stackSpace = (sL - sB); }
    else { stackSpace = (sB - sL); }
    LogInfo("Available stack space: %zu bytes.\n", stackSpace);
}
#endif

void Platform_Sleep(u32 ms)
{
    Sleep(ms);
}

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Args;
};

struct platform_work_queue
{
    // How many entries of the worker should be completed.
    u32 volatile CompletionGoal;
    // How manyu tasks have been completed.
    u32 volatile CompletionCount;
    // If a new work entry, which index in the queue array should it go to?
    u32 volatile NextEntryToWrite;
    u32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;

    // Queue Size.
    platform_work_queue_entry Entries[256];
};

static b32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    b32 WeShouldSleep = false;

    // Work queue is a Circular buffer.
    // NOTE: we do not want two threads to read the same entry (in other words, we dont want two threads
    // doing the same work).
    // What is the next index in queue array that we should pick to complete using a thread?
    u32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    // What will be the next index of the work to pick after the thread has picked the original one?
    u32 NewNextEntryToRead = ((OriginalNextEntryToRead + 1) % ARRAY_SIZE(Queue->Entries));

    // NextEntryToRead is the index of work entries which need to be picked up by threads for completion.
    // Next Entry to Write is the index where new work if requested will be added to.
    // We dont want a thread to pick up work that we are or will be writing to.
    // Either it is in the process of being written to
    if (OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        // Compare the value of NextEntryToRead with OriginalNextEntryToRead, if its different then replace it with
        // NewNextEntryToRead and return the original expected value, otherwise do nothing.
        //
        // NOTE: Take this scenario where this will fail.
        // Let's say thread A reached here, it had an idea of what the OriginalNextEntryToRead was.
        // Then our OS scheduled another thread B which did the same thing, got the same value for OriginalNextEntryToRead.
        // It exchanged it and set Queue's NextEntryToRead value to Original+1.
        //
        // Now let's say thread A again got scheduled and it started from this line. Now, Queue's NextEntryRead has
        // changed to Original+1, and it had the old Original value. In this case InterlockedCompareExchange will
        // tell thread A that it was changed since the last time you stored Original's value. thread B now knows
        // some other thread already updated the value, in which case it should not pick the same work again to
        // complete that thread B already picked up. so it will do nothing and continue checking for more qork from
        // the queue.
        u32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead, NewNextEntryToRead,
                                               OriginalNextEntryToRead);
        // Index will be the Original value since the original value will be returned if the exchange did happen
        // safely above.
        if (Index == OriginalNextEntryToRead)
        {
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Args);
            // Thread Safe way to increment CompletionCount.
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        // This will come when OriginalNextEntryToRead will be equal to that of Write.
        // which essentially means there is no work left in the queue. In which case the thread is instructed to
        // sleep until some new work gets added to the queue.
        WeShouldSleep = true;
    }

    return WeShouldSleep;
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    platform_work_queue *Queue = (platform_work_queue *)lpParameter;

#if __MS10
    // NOTE: Setting thread name for debugging later in vs2022
    // wchar_t Buffer[256];
    // ConstructWideString(Buffer, ArrayCount(Buffer), L"Thread %d", ThreadInfo->LogicalThreadIndex);
    // HRESULT r = SetThreadDescription(GetCurrentThread(), (PCWSTR)Buffer);
#endif

    for (;;)
    {
        if (Win32DoNextWorkQueueEntry(Queue))
        {
            // Sleep Infinitely until the semaphore is released.
            // Semaphore is released when there is work in the queue to be completed.
            WaitForSingleObjectEx(Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
}

static void
Win32MakeWorkQueue(platform_work_queue *Queue, u32 ThreadCount)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;

    u32 InitialCount = 0;
    Queue->SemaphoreHandle = CreateSemaphoreExA(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);

    for(u32 ThreadIndex = 0; ThreadIndex < ThreadCount; ++ThreadIndex)
    {
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Queue, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

void
Platform_AddWorkEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // The Thread Work Queue is a circular buffer. Write pointer can get
    // ahead of the read as it wraps areound the entries array.
    //
    // if some new work comes in, which index should we choose to store the work related info. That's the
    // NextEntryToWrite.
    u32 NewNextEntryToWrite = ((Queue->NextEntryToWrite + 1) % ARRAY_SIZE(Queue->Entries));
    // NextEntryToRead is where the next available thread will pick from to complete.
    // We dont want to write data to the same place and simulataneously have a thread picking it to complete.
    ASSERT(NewNextEntryToWrite != Queue->NextEntryToRead);

    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;

    // Data related to the work entry
    Entry->Args = Data;
    Entry->Callback = Callback;

    // Thread safe way of incrementing completion goal.
    InterlockedIncrement(&Queue->CompletionGoal);

    CompletePastWritesBeforeFutureWrites;

    // Circular buffer.
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    // Release 1 count for the semaphore so that sleeping threads are woken up to start doing work for this newly
    // added entry.
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

void
Platform_CompleteAllWork(platform_work_queue *Queue)
{
    while (Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }

    // NOTE: Reset the Queue.
    Queue->CompletionCount = 0;
    Queue->CompletionGoal = 0;
}

static
PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
    LogInfo("Thread: %u, Data: %s\n", GetCurrentThreadId(), (char *)Args);
}

#if 0
platform_mutex::platform_mutex()
    : MutexHandle(CreateMutex(0, FALSE, 0))
{
    if(MutexHandle == NULL)
    {
        LogFatalUnformatted("Mutex Handle could not be created!\n");
    }
}

platform_mutex::~platform_mutex() { CloseHandle(MutexHandle); }

void
platform_mutex::Lock()
{
    DWORD Result = WaitForSingleObject(MutexHandle, INFINITE);
    if(Result != WAIT_OBJECT_0)
    {
        LogFatalUnformatted("Problem while Locking\n");
    }
}

void platform_mutex::Unlock()
{
    if(!ReleaseMutex(MutexHandle))
    {
        LogFatalUnformatted("Trouble releasing mutex!\n");
    }
}
#endif

void
Platform_FreeFileMemory(platform_read_file_result *File)
{
    if (File->Data)
    {
        VirtualFree(File->Data, 0, MEM_RELEASE);
        File->Size = 0;
    }
    File->Path = nullptr;
}

platform_read_file_result
Platform_ReadFile(const char *Path)
{
    platform_read_file_result Result = {};
    ASSERT(Path != nullptr);

    HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            ASSERT(FileSize.QuadPart <= 0xffffffff);
            u32 FileSize32 = (u32)FileSize.QuadPart;

            Result.Data = (u8 *)VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Data)
            {
                    DWORD BytesRead = 100;
                    if (ReadFile(FileHandle, Result.Data, FileSize32, &BytesRead, 0))
                    {
                        ASSERT(BytesRead == FileSize32);
                        Result.Size = FileSize32;
                        Result.Path = const_cast<char *>(Path);
                    }
                    else
                    {
                        Platform_FreeFileMemory(&Result);
                    }
            }
        }

        CloseHandle(FileHandle);
    }

    return Result;
}

b32
Platform_WriteFile(char *Filename, u32 Size, void *Data)
{
    b32 Result = false;
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Data, Size, &BytesWritten, 0))
        {
            ASSERT(Size == BytesWritten);
            Result = true;
        }
        else
        {
            Result = false;
        }
        CloseHandle(FileHandle);
    }

    return Result;
}

u64
Platform_GetFileTime()
{
    FILETIME Filetime;
    GetSystemTimeAsFileTime(&Filetime);

    ULARGE_INTEGER ULI;
    ULI.LowPart = Filetime.dwLowDateTime;
    ULI.HighPart = Filetime.dwHighDateTime;

    return ULI.QuadPart;
}

u64
GetPerfCounterValue()
{
    LARGE_INTEGER PerfCounter;
    QueryPerformanceCounter(&PerfCounter);

    return PerfCounter.QuadPart;
}

u32
Platform_GetRandomSeed()
{
    u64 Time = Platform_GetFileTime();
    u64 PerfCounter = GetPerfCounterValue();
    u32 ProcessId = GetCurrentProcessId();
    u32 ThreadId = GetCurrentThreadId();

    // Combine them all
    u64 CombinedSeed = Time ^ PerfCounter ^ ((u64)ProcessId << 32) ^ ThreadId;

    CombinedSeed = (CombinedSeed ^ (CombinedSeed >> 30)) * 0xbf58476d1ce4e5b9ULL;
    CombinedSeed = (CombinedSeed ^ (CombinedSeed >> 27)) * 0x94d049bb133111ebULL;
    CombinedSeed = CombinedSeed ^ (CombinedSeed >> 31);

    u32 Result = (u32)(CombinedSeed & 0xffffffff);
    return Result;
}

void
Platform_GetWindowDetails(void **WindowHandle, void **WindowInstance)
{
    *WindowHandle = Win32State.WindowContext.Handle;
    *WindowInstance = Win32State.WindowContext.hInstance;
}

// TODO)): Right now this is the only entry point since win32 is the only platform right now.
// TODO)): Have to implement multiple entrypoints for all platforms.
i32 WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int CmdShow)
{
    platform_work_queue HighPriorityQueue = {};
    Win32MakeWorkQueue(&HighPriorityQueue, 7);

    platform_work_queue LowPriorityQueue = {};
    Win32MakeWorkQueue(&LowPriorityQueue, 3);

#if 0
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A0.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A1.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A2.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A3.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A4.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A5.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A6.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A7.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A8.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String A9.");
    Sleep(5000);
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B0.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B1.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B2.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B3.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B4.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B5.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B6.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B7.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B8.");
    Platform_AddWorkEntry(&HighPriorityQueue, DoWorkerWork, (void *)"String B9.");

    Platform_CompleteAllWork(&HighPriorityQueue);
#endif

#if SHU_CRASH_DUMP_ENABLE
    if (Win32State.WriteFp == nullptr)
    {
        WriteFp = fopen("app_dump.txt", "w");
        fclose(WriteFp);
    }
#endif
    // OutputToConsole(LogType_Info, "Inside WinMain\n");

    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    Win32State.PerfFrequency = Frequency.QuadPart;

    // TODO)): Re-Enable this after removing FPS Capping to specific values.
#if 1
    DEVMODE DM;
    DM.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &DM);
    Win32State.MonitorRefreshRate = DM.dmDisplayFrequency;
    Win32State.FPS = Win32State.MonitorRefreshRate;
    f32 TargetMS = 1000.0f / (f32)Win32State.FPS;
#endif
#if FPS_CAPPING_ENABLED
    TIMECAPS TimeCaps;
    MMRESULT TimerResult = timeGetDevCaps(&TimeCaps, sizeof(TIMECAPS));
    u32 MinSleepGranularity = TimeCaps.wPeriodMin;
#endif

    b32 CreateConsole = ShouldCreateConsole(pCmdLine);

    if(CreateConsole)
    {
        Win32SetConsoleHandle();
    }

    const wchar_t CLASS_NAME[] = L"Shoora Engine";

    WNDCLASS WinClass = {};
    WinClass.lpfnWndProc = Win32WindowCallback;
    WinClass.hCursor = LoadCursor(0, IDC_ARROW);
    WinClass.hInstance = hInstance;
    WinClass.lpszClassName = CLASS_NAME;

    RegisterClass(&WinClass);

    DWORD Style = WS_OVERLAPPEDWINDOW;
    u32 StartingWindowWidth = 1600;
    u32 StartingWindowHeight = 900;
    HWND WindowHandle = CreateWindowEx(0, CLASS_NAME, L"Shoora", Style, CW_USEDEFAULT, CW_USEDEFAULT,
                                       StartingWindowWidth, StartingWindowHeight, NULL, NULL, hInstance, NULL);

    AppInfo.JobQueue = &HighPriorityQueue;

#if _SHU_DEBUG
    LPVOID BaseAddress = (LPVOID) TERABYTES(2);
#else
    LPVOID BaseAddress = 0;
#endif
    AppInfo.GameMemory.PermSize = MEGABYTES(512);
    AppInfo.GameMemory.FrameMemorySize = GIGABYTES(1);

    Win32State.MemoryBlockSize = AppInfo.GameMemory.PermSize +
                                 AppInfo.GameMemory.FrameMemorySize;
    Win32State.MemoryBlock = VirtualAlloc(BaseAddress, Win32State.MemoryBlockSize, MEM_RESERVE | MEM_COMMIT,
                                          PAGE_READWRITE);

    AppInfo.GameMemory.PermMemory = Win32State.MemoryBlock;
    AppInfo.GameMemory.FrameMemory = (void *)((u8 *)AppInfo.GameMemory.PermMemory +
                                                          AppInfo.GameMemory.PermSize);

    Win32State.WindowContext.Handle = WindowHandle;
    Win32State.WindowContext.ClearColor = CreateSolidBrush(RGB(48, 10, 36));
    Win32State.WindowContext.hInstance = hInstance;

    ShowWindow(WindowHandle, CmdShow);

    platform_input_state InputStates[2] = {};
    platform_input_state *OldInputState = InputStates;
    platform_input_state *NewInputState = InputStates + 1;

    // TODO)): Get the AppName from the game dll.
    InitializeRenderer(&AppInfo);

    Win32State.Running = true;
    LARGE_INTEGER FrameMarkerStart = Win32GetWallClock();
    LARGE_INTEGER FrameMarkerEnd = Win32GetWallClock();

    while (Win32State.Running)
    {
        Win32State.InputState = NewInputState;

        // MOUSE
        u32 MouseButtonsKeyCodes[5] =
        {
            SU_LEFTMOUSEBUTTON,
            SU_RIGHTMOUSEBUTTON,
            SU_MIDDLEMOUSEBUTTON,
            SU_SPECIALMOUSEBUTTON1,
            SU_SPECIALMOUSEBUTTON2,
        };

        // Mouse
        POINT ClientRelativeMousePos;
        GetCursorPos(&ClientRelativeMousePos);
        ScreenToClient(WindowHandle, &ClientRelativeMousePos);
        NewInputState->MouseXPos = ClientRelativeMousePos.x;
        NewInputState->MouseYPos = ClientRelativeMousePos.y;
        if(NewInputState->Buttons[SU_RIGHTMOUSEBUTTON].IsCurrentlyDown)
        {
            Win32SetOutOfBoundsCursor(WindowHandle);
        }

        for(i32 MouseButtonIndex = 0;
            MouseButtonIndex < ARRAY_SIZE(MouseButtonsKeyCodes);
            ++MouseButtonIndex)
        {
            u32 kc = MouseButtonsKeyCodes[MouseButtonIndex];

            NewInputState->Buttons[kc].IsCurrentlyDown = OldInputState->Buttons[kc].IsCurrentlyDown;
            NewInputState->Buttons[kc].IsReleased = OldInputState->Buttons[kc].IsReleased;
            NewInputState->Buttons[kc].ButtonTransitionsPerFrame = 0;

            b32 IsButtonDown = (GetAsyncKeyState(MouseButtonsKeyCodes[MouseButtonIndex]) & KEY_PRESS_MASK) != 0;
            Win32UpdateInputButtonState(&NewInputState->Buttons[kc], IsButtonDown);
        }

        // KEYBOARD
        for(i32 KeyboardKeycode = SHU_FIRST_KEYBOARD_KEYCODE;
            KeyboardKeycode < SU_KEYCODE_MAX;
            ++KeyboardKeycode)
        {
            NewInputState->Buttons[KeyboardKeycode].IsCurrentlyDown = OldInputState->Buttons[KeyboardKeycode].IsCurrentlyDown;
            NewInputState->Buttons[KeyboardKeycode].IsReleased = false;
            NewInputState->Buttons[KeyboardKeycode].ButtonTransitionsPerFrame = 0;
        }
        Win32ProcessWindowsMessageQueue(Win32State.WindowContext.Handle, NewInputState);

        shoora_platform_frame_packet FramePacket = {};

        FramePacket.MouseXPos = NewInputState->MouseXPos;
        FramePacket.MouseYPos = NewInputState->MouseYPos;

        f32 DeltaTime = Win32GetSecondsElapsed(FrameMarkerStart, FrameMarkerEnd);
        if(DeltaTime <= 0.0f) { DeltaTime = 1.0f; }
        u32 FPS = (u32)(1.0f / DeltaTime);

        FramePacket.DeltaTime = DeltaTime;
        FramePacket.Fps = FPS;

        // NOTE: Game Update
        DrawFrame(&FramePacket);

        platform_input_state *Temp = NewInputState;
        NewInputState = OldInputState;
        OldInputState = Temp;

        FrameMarkerStart = FrameMarkerEnd;
        FrameMarkerEnd = Win32GetWallClock();

        if (Win32State.IsFPSCap && (DeltaTime > 0.0f))
        {
            f32 FrameMS = DeltaTime*1000.0f;
            TargetMS = 1000.0f / (f32)Win32State.FPS;
#if FPS_CAPPING_ENABLED
            // if(CreateConsole) LogInfo("FrameMS: %f, TargetMS: %f\n", FrameMS, TargetMS);
            if(TargetMS > FrameMS)
            {
                u32 SleepMS = (u32)(TargetMS - FrameMS);
                if(SleepMS > MinSleepGranularity)
                {
                    // if(CreateConsole) LogTrace("Sleeping for %u milliseconds.\n", SleepMS);
                    timeBeginPeriod(TimeCaps.wPeriodMin);
                    Sleep(SleepMS);
                    timeEndPeriod(TimeCaps.wPeriodMin);
                }
            }
#endif
        }
    }

    DestroyRenderer();
    CloseWindow(Win32State.WindowContext.Handle);

    if(CreateConsole)
    {
        Win32PauseConsoleWindow();
    }

    if (Win32State.MemoryBlock != nullptr)
    {
        VirtualFree(Win32State.MemoryBlock, 0, MEM_RELEASE);
        Win32State.MemoryBlock = nullptr;
    }

#if SHU_CRASH_DUMP_ENABLE
    if (Win32State.WriteFp != nullptr)
    {
        fclose(Win32State.WriteFp);
        Win32State.WriteFp = nullptr;
    }
#endif
    return 0;
}
