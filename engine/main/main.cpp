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
    b32 IsDown;
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
        platform_input_button_state KeyboardButtons[9];

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
        };
    };
};



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
    if(InputButtonState->IsDown != IsCurrentlyDown)
    {
        ++InputButtonState->ButtonTransitionsPerFrame;
        InputButtonState->IsDown = IsCurrentlyDown;
    }
}

b32
Win32InputKeyPressed(platform_input_button_state InputState)
{
    b32 Result = ((InputState.ButtonTransitionsPerFrame > 1) ||
                  (InputState.ButtonTransitionsPerFrame == 1 && InputState.IsDown));

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
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            break;
        }
        else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
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
    char Buffer[1024];
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

                b8 KeyWasPreviouslyDown = ((Message.lParam & (1 << 30)) != 0);
                b8 KeyIsCurrentlyDown = ((Message.lParam & (1 << 31)) == 0);

                b8 KeyIsPressed = KeyIsCurrentlyDown &&
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

// TODO)): Right now this is the only entry point since win32 is the only platform right now.
// TODO)): Have to implement multiple entrypoints for all platforms.
i32 WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int CmdShow)
{
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

    platform_input_state InputState[2] = {};
    platform_input_state *OldInputState = InputState;
    platform_input_state *NewInputState = InputState + 1;

    renderer_context RendererContext = {};
    // TODO)): Get the AppName from the game dll.
    InitializeRenderer(&RendererContext, &AppInfo);

    Win32GlobalRunning = true;
    while(Win32GlobalRunning)
    {
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

        for(i32 ButtonIndex = 0;
            ButtonIndex < 5;
            ++ButtonIndex)
        {
                NewInputState->MouseButtons[ButtonIndex].IsDown = OldInputState->MouseButtons[ButtonIndex].IsDown;
                NewInputState->MouseButtons[ButtonIndex].ButtonTransitionsPerFrame = 0;

                b32 IsButtonDown = GetKeyState(MouseButtonsKeyCodes[ButtonIndex]) & (1 << 15);
                Win32UpdateInputButtonState(&NewInputState->MouseButtons[ButtonIndex], IsButtonDown);
        }

        // KEYBOARD
        for(i32 ButtonIndex = 0;
            ButtonIndex < ARRAY_SIZE(NewInputState->KeyboardButtons);
            ++ButtonIndex)
        {
            NewInputState->KeyboardButtons[ButtonIndex].IsDown = OldInputState->KeyboardButtons[ButtonIndex].IsDown;
            NewInputState->KeyboardButtons[ButtonIndex].ButtonTransitionsPerFrame = 0;
        }
        Win32ProcessWindowsMessageQueue(GlobalWin32WindowContext.Handle, NewInputState);

        // NOTE: Input Key Press Check
        // if(Win32InputKeyPressed(NewInputState->Keyboard_W))
        // {
        //     OutputDebugString(L"W was pressed!\n");
        // }
        if(Win32InputKeyPressed(NewInputState->MouseButtons[0]))
        {
            WIN32_LOG_OUTPUT("Left Mouse Button was pressed at [%f, %f]!\n",
                             NewInputState->MouseXPos, NewInputState->MouseYPos);
        }


        platform_input_state *Temp = NewInputState;
        NewInputState = OldInputState;
        OldInputState = Temp;

        // NOTE: Game Update
        DrawFrame();
    }

    DestroyRenderer(&RendererContext);
    CloseWindow(GlobalWin32WindowContext.Handle);

    if(CreateConsole)
    {
        Win32PauseConsoleWindow();
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