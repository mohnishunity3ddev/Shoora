#ifndef UNICODE
#define UNICODE
#endif

#include "defines.h"
#include "platform/platform.h"
#include "platform/windows/win_platform.h"
#include <Windows.h>
#include <shellapi.h>

#include "renderer/renderer_frontend.h"

// TODO)): Remove this and implement your own!
#include <stdio.h>

struct win32_window_context
{
    HWND Handle;
    HBRUSH ClearColor;
    HANDLE ConsoleHandle;
};
static win32_window_context GlobalWin32WindowContext = {};
static WINDOWPLACEMENT GlobalWin32WindowPosition = {sizeof(GlobalWin32WindowPosition)};
static b8 Win32GlobalRunning = false;
static int64 Win32GlobalPerfFrequency = 0;
static FILE *WriteFp = nullptr;

void DummyWinResize(u32 Width, u32 Height) {}

static shoora_app_info AppInfo =
{
    .AppName = "Placeholder App Name",
    .WindowResizeCallback = &DummyWinResize
};

RECT
Win32GetWindowRect(HWND WindowHandle)
{
    RECT ClientRect;
    GetClientRect(WindowHandle, &ClientRect);

    return ClientRect;
}

void
Win32ToggleFullscreen(HWND Window)
{
    // NOTE: This follows Raymond Chens prescription for fullscreen toggling.
    // see : https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if (GetWindowPlacement(Window, &GlobalWin32WindowPosition) &&
            GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP, MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWin32WindowPosition);
        SetWindowPos(Window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

struct platform_input_button_state
{
    i32 ButtonTransitionsPerFrame;
    b32 IsCurrentlyDown;
    b32 IsReleased;
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

struct platform_input_state
{
    platform_input_button_state MouseButtons[MouseButton_Count];
    f32 MouseXPos, MouseYPos;

    union
    {
        platform_input_button_state KeyboardButtons[10];

        struct
        {
            platform_input_button_state Keyboard_W;
            platform_input_button_state Keyboard_A;
            platform_input_button_state Keyboard_S;
            platform_input_button_state Keyboard_D;
            platform_input_button_state Keyboard_UpArrow;
            platform_input_button_state Keyboard_DownArrow;
            platform_input_button_state Keyboard_LeftArrow;
            platform_input_button_state Keyboard_RightArrow;
            platform_input_button_state Keyboard_Enter;
            platform_input_button_state Keyboard_LeftShift;
        };
    };
};
static platform_input_state *GlobalInputState = nullptr;

LRESULT CALLBACK
Win32WindowCallback(HWND WindowHandle, UINT Message, WPARAM WParam, LPARAM LParam)
{
    LRESULT Result = 0;
    switch(Message)
    {
        case WM_QUIT:
        case WM_CLOSE:
        case WM_DESTROY:
        {
            Win32GlobalRunning = false;
        } break;

        case WM_SIZE:
        {
            i32 Width = LOWORD(LParam);
            i32 Height = HIWORD(LParam);
            AppInfo.WindowWidth = Width;
            AppInfo.WindowHeight = Height;
            AppInfo.WindowResizeCallback(Width, Height);
        } break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            ASSERT(!"Keyboard input came in here through a non-dispatch message!");
            // NOTE: Moved the actual keyboard handling code down in the main WinMain loop.
        }
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT PaintStruct;
            HDC DC = BeginPaint(WindowHandle, &PaintStruct);

            RECT ClientRect = Win32GetWindowRect(WindowHandle);
            FillRect(DC, &ClientRect, GlobalWin32WindowContext.ClearColor);

            ReleaseDC(WindowHandle, DC);
            EndPaint(WindowHandle, &PaintStruct);
        } break;

        default:
        {
            Result = DefWindowProc(WindowHandle, Message, WParam, LParam);
        }
    }

    return Result;
}

void
Win32UpdateInputButtonState(platform_input_button_state *InputButtonState, b32 IsCurrentlyDown)
{
    if(InputButtonState->IsCurrentlyDown != IsCurrentlyDown)
    {
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

b8
Platform_GetKeyInputState(u8 KeyCode, KeyState State)
{
    b8 Result = false;
    switch(State)
    {
        case SHU_KEYSTATE_PRESS:
        {

        } break;

        case SHU_KEYSTATE_DOWN:
        {
            if(KeyCode == 'W' && GlobalInputState->Keyboard_W.IsCurrentlyDown)
            {
                Result = true;
            }
            else if(KeyCode == 'A' && GlobalInputState->Keyboard_A.IsCurrentlyDown)
            {
                Result = true;
            }
            else if(KeyCode == 'S' && GlobalInputState->Keyboard_S.IsCurrentlyDown)
            {
                Result = true;
            }
            else if(KeyCode == 'D' && GlobalInputState->Keyboard_D.IsCurrentlyDown)
            {
                Result = true;
            }
            else if(KeyCode == SU_LEFTSHIFT && GlobalInputState->Keyboard_LeftShift.IsCurrentlyDown)
            {
                Result = true;
            }
        } break;

        case SHU_KEYSTATE_RELEASE:
        {
            if (KeyCode == 'W' && GlobalInputState->Keyboard_W.IsReleased)
            {
                Result = true;
            }
            else if (KeyCode == 'A' && GlobalInputState->Keyboard_A.IsReleased)
            {
                Result = true;
            }
            else if (KeyCode == 'S' && GlobalInputState->Keyboard_S.IsReleased)
            {
                Result = true;
            }
            else if (KeyCode == 'D' && GlobalInputState->Keyboard_D.IsReleased)
            {
                Result = true;
            }
            else if (KeyCode == SU_LEFTSHIFT && GlobalInputState->Keyboard_LeftShift.IsReleased)
            {
                Result = true;
            }
        }
        break;

        default:
        {
            LogWarn("KeyState (%u) for Key(%u) was not identified!", (u32)State, (u32)KeyCode);
        } break;
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

    if(Result != 0)
    {
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
    HANDLE Console = GlobalWin32WindowContext.ConsoleHandle;

    if(Console == 0)
    {
        Console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (Console == 0)
        {
            // If GetStdHandle fails, allocate a new console
            AllocConsole();
            Console = GetStdHandle(STD_OUTPUT_HANDLE);
            if (Console == 0)
            {
                // If GetStdHandle still fails after allocation, log the error and return
                Win32LogLastError();
                return;
            }
            else
            {
                GlobalWin32WindowContext.ConsoleHandle = Console;
            }
        }
        else
        {
            GlobalWin32WindowContext.ConsoleHandle = Console;
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
        if (GetKeyState(VK_RETURN) & (1 << 15))
        {
            break;
        }
        else if (GetKeyState(VK_ESCAPE) & (1 << 15))
        {
            break;
        }

        Sleep(100);
    }
}


void
OutputDebugStringColor(LogType LogType, const char *message)
{
    static COLORREF Colors[] = {RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255), RGB(242, 188, 47),
                                RGB(40, 130, 255), RGB(128, 128, 128), RGB(96, 96, 96)};
    COLORREF Color = Colors[4];
    switch(LogType)
    {
        case LogType_DebugReportCallbackInfo:
        {
            Color = Colors[6];
        }
        break;
        case LogType_ValidationLayerInfo:
        {
            Color = Colors[5];
        } break;
        case LogType_Fatal:
        case LogType_Error:
        {
            Color = Colors[0];
        } break;
        case LogType_Warn:
        {
            Color = Colors[3];
        } break;
        case LogType_Info:
        case LogType_Debug:
        case LogType_Trace:
        default:
        {
        } break;
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
    WriteFp = fopen("app_dump.txt", "a");
    fprintf(WriteFp, "%s", Message);
    fclose(WriteFp);
    WriteFp = nullptr;
#else
    fprintf(WriteFp, "%s", Message);
#endif

    HANDLE Console = GlobalWin32WindowContext.ConsoleHandle;

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
void
LogInfo(const char *Format, ...)
{
    LOG_FORMAT(LogType_Info, Format);
}
void
LogDebug(const char *Format, ...)
{
    LOG_FORMAT(LogType_Debug, Format);
}
void
LogWarn(const char *Format, ...)
{
    LOG_FORMAT(LogType_Warn, Format);
}
void
LogError(const char *Format, ...)
{
    LOG_FORMAT(LogType_Error, Format);
}
void
LogFatal(const char *Format, ...)
{
    LOG_FORMAT(LogType_Fatal, Format);
}
void
LogTrace(const char *Format, ...)
{
    LOG_FORMAT(LogType_Trace, Format);
}

void
LogInfoUnformatted(const char *Message)
{
    OutputToConsole(LogType_Info, Message);
}
void
LogDebugUnformatted(const char *Message)
{
    OutputToConsole(LogType_Debug, Message);
}
void
LogWarnUnformatted(const char *Message)
{
    OutputToConsole(LogType_Warn, Message);
}
void
LogErrorUnformatted(const char *Message)
{
    OutputToConsole(LogType_Error, Message);
}
void
LogFatalUnformatted(const char *Message)
{
    OutputToConsole(LogType_Fatal, Message);
}
void
LogTraceUnformatted(const char *Message)
{
    OutputToConsole(LogType_Trace, Message);
}

void
Platform_ExitApplication(const char *Reason)
{
    LogOutput(LogType_Fatal, Reason);

    Win32GlobalRunning = false;
}

void
LogString(const char *String)
{
    OutputDebugStringA(String);
}

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

                if(KeyCode == SU_ESCAPE && KeyIsPressed)
                {
                    Win32GlobalRunning = false;
                }
                else if(KeyCode == 'W')
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_W, KeyIsCurrentlyDown);
                }
                else if(KeyCode == 'A')
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_A, KeyIsCurrentlyDown);
                }
                else if(KeyCode == 'S')
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_S, KeyIsCurrentlyDown);
                }
                else if(KeyCode == 'D')
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_D, KeyIsCurrentlyDown);
                }
                else if(KeyCode == SU_UPARROW)
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_UpArrow, KeyIsCurrentlyDown);
                }
                else if(KeyCode == SU_DOWNARROW)
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_DownArrow, KeyIsCurrentlyDown);
                }
                else if(KeyCode == SU_LEFTARROW)
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_LeftArrow, KeyIsCurrentlyDown);
                }
                else if(KeyCode == SU_RIGHTARROW)
                {
                    Win32UpdateInputButtonState(&Input->Keyboard_RightArrow, KeyIsCurrentlyDown);
                }
                else
                {
                    b32 ShiftKeyDown = (GetKeyState(SU_LEFTSHIFT) & (1 << 15));
                    Win32UpdateInputButtonState(&Input->Keyboard_LeftShift, ShiftKeyDown ? true : false);

                    // GGetAsyncKeyState returns the state of the key RIGHT NOW! even if previously keys were
                    // pressed and were not handled.
                    b32 WasAltKeyDown = (GetKeyState(SU_ALT) & (1 << 15));
                    if(WasAltKeyDown && KeyIsPressed)
                    {
                        // Alt + F4
                        if(KeyCode == SU_F4)
                        {
                            Win32GlobalRunning = false;
                        }
                        // Alt + Enter
                        else if(KeyCode == SU_RETURN)
                        {
                            Win32ToggleFullscreen(Message.hwnd);
                        }
                    }

                    if(WasAltKeyDown && ShiftKeyDown && KeyIsPressed)
                    {
                        if(KeyCode == SU_KEY1)
                        {
                            LogErrorUnformatted("Shift + Alt + 1 was pressed!\n");
                        }
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
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)Win32GlobalPerfFrequency);
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

// TODO)): Right now this is the only entry point since win32 is the only platform right now.
// TODO)): Have to implement multiple entrypoints for all platforms.
i32 WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int CmdShow)
{
    if (WriteFp == nullptr)
    {
        WriteFp = fopen("app_dump.txt", "w");
#if SHU_CRASH_DUMP_ENABLE
        fclose(WriteFp);
#endif
    }
    OutputToConsole(LogType_Info, "Inside WinMain\n");

    LARGE_INTEGER Frequency;
    QueryPerformanceFrequency(&Frequency);
    Win32GlobalPerfFrequency = Frequency.QuadPart;

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

    RECT WindowRect = Win32GetWindowRect(WindowHandle);
    AppInfo.WindowWidth = WindowRect.right - WindowRect.left;
    AppInfo.WindowHeight = WindowRect.bottom - WindowRect.top;

    GlobalWin32WindowContext.Handle = WindowHandle;
    GlobalWin32WindowContext.ClearColor = CreateSolidBrush(RGB(48, 10, 36));

    ShowWindow(WindowHandle, CmdShow);

    platform_input_state InputStates[2] = {};
    platform_input_state *OldInputState = InputStates;
    platform_input_state *NewInputState = InputStates + 1;

    renderer_context RendererContext = {};
    // TODO)): Get the AppName from the game dll.
    InitializeRenderer(&RendererContext, &AppInfo);

    Win32GlobalRunning = true;
    LARGE_INTEGER FrameMarkerStart = Win32GetWallClock();
    LARGE_INTEGER FrameMarkerEnd = Win32GetWallClock();
    while(Win32GlobalRunning)
    {
        GlobalInputState = NewInputState;

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
        // Win32SetOutOfBoundsCursor(WindowHandle);

        for(i32 ButtonIndex = 0;
            ButtonIndex < 5;
            ++ButtonIndex)
        {
                NewInputState->MouseButtons[ButtonIndex].IsCurrentlyDown = OldInputState->MouseButtons[ButtonIndex].IsCurrentlyDown;
                NewInputState->MouseButtons[ButtonIndex].ButtonTransitionsPerFrame = 0;

                b32 IsButtonDown = GetKeyState(MouseButtonsKeyCodes[ButtonIndex]) & (1 << 15);
                Win32UpdateInputButtonState(&NewInputState->MouseButtons[ButtonIndex], IsButtonDown);
        }

        // KEYBOARD
        for(i32 ButtonIndex = 0;
            ButtonIndex < ARRAY_SIZE(NewInputState->KeyboardButtons);
            ++ButtonIndex)
        {
            NewInputState->KeyboardButtons[ButtonIndex].IsCurrentlyDown = OldInputState->KeyboardButtons[ButtonIndex].IsCurrentlyDown;
            NewInputState->KeyboardButtons[ButtonIndex].IsReleased = false;
            NewInputState->KeyboardButtons[ButtonIndex].ButtonTransitionsPerFrame = 0;
        }
        Win32ProcessWindowsMessageQueue(GlobalWin32WindowContext.Handle, NewInputState);

#if 0
        // NOTE: Input Key Press Check
        if(Win32InputKeyPressed(&NewInputState->Keyboard_W))
        {
            LogInfoUnformatted("W was pressed!\n");
        }
        if(NewInputState->Keyboard_W.IsReleased)
        {
            LogInfoUnformatted("A is Released!\n");
        }
        if(NewInputState->Keyboard_A.IsCurrentlyDown)
        {
            LogInfoUnformatted("A is Down!\n");
        }
        if(NewInputState->Keyboard_A.IsReleased)
        {
            LogInfoUnformatted("A is Released!\n");
        }
        if(NewInputState->Keyboard_LeftShift.IsCurrentlyDown)
        {
            LogInfoUnformatted("Keyboard_LeftShift is Down!\n");
        }
        if(NewInputState->Keyboard_LeftShift.IsReleased)
        {
            LogInfoUnformatted("Left Shift is Released!\n");
        }
#endif

        shoora_platform_frame_packet FramePacket = {};

        // TODO)): Refactor this into an array. or get the mouse button info from the actual code which wants it.
        if(Win32InputKeyPressed(&NewInputState->MouseButtons[0]))
        {
            FramePacket.LeftMouseClicked = true;
            // WIN32_LOG_OUTPUT("Left Mouse Button was pressed at [%f, %f]!\n",
            //                  NewInputState->MouseXPos, NewInputState->MouseYPos);
        }
        FramePacket.IsLeftMouseDown = NewInputState->MouseButtons[0].IsCurrentlyDown;
        FramePacket.MouseXPos = NewInputState->MouseXPos;
        FramePacket.MouseYPos = NewInputState->MouseYPos;

        f32 DeltaTime = Win32GetSecondsElapsed(FrameMarkerStart, FrameMarkerEnd);
        if(DeltaTime <= 0.0f)
        {
            DeltaTime = 1.0f;
        }
        u32 FPS = (u32)(1.0f / DeltaTime);
        // LogOutput(LogType_Trace, "DeltaTime: %f, FPS: %u\n", DeltaTime, FPS);

        FramePacket.DeltaTime = DeltaTime;
        FramePacket.Fps = FPS;

        // NOTE: Game Update
        DrawFrame(&FramePacket);

        platform_input_state *Temp = NewInputState;
        NewInputState = OldInputState;
        OldInputState = Temp;

        FrameMarkerStart = FrameMarkerEnd;
        FrameMarkerEnd = Win32GetWallClock();
    }

    DestroyRenderer(&RendererContext);
    CloseWindow(GlobalWin32WindowContext.Handle);

    if(CreateConsole)
    {
        Win32PauseConsoleWindow();
    }


    if (WriteFp != nullptr)
    {
        fclose(WriteFp);
        WriteFp = nullptr;
    }
    return 0;
}

#ifdef SHU_RENDERER_BACKEND_VULKAN
void
FillVulkanWin32SurfaceCreateInfo(shoora_platform_presentation_surface *Surface)
{
#ifdef WIN32
    VkWin32SurfaceCreateInfoKHR *SurfaceCreateInfo = Surface->Win32SurfaceCreateInfo;

    SurfaceCreateInfo->sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    SurfaceCreateInfo->pNext = 0;
    SurfaceCreateInfo->flags = 0;
    SurfaceCreateInfo->hinstance = GetModuleHandle(0);
    SurfaceCreateInfo->hwnd = GlobalWin32WindowContext.Handle;
#endif
}
#endif