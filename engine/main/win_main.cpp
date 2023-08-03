#ifndef UNICODE
#define UNICODE
#endif

#include "defines.h"
#include "win_platform.h"
#include <Windows.h>

#include "renderer/renderer_frontend.h"

// TODO)): Remove this and implement your own!
#include <stdio.h>

struct win32_window_context
{
    HWND Handle;
    HBRUSH ClearColor;
};
static win32_window_context GlobalWin32WindowContext = {};
static WINDOWPLACEMENT GlobalWin32WindowPosition = {sizeof(GlobalWin32WindowPosition)};
static b8 Win32GlobalRunning = false;

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
#if 0
            i32 Width = LOWORD(LParam);
            i32 Height = HIWORD(LParam);
#endif
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

int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int CmdShow)
{
    const wchar_t CLASS_NAME[] = L"Shura Engine";

    WNDCLASS WinClass = {};
    WinClass.lpfnWndProc = Win32WindowCallback;
    WinClass.hCursor = LoadCursor(0, IDC_ARROW);
    WinClass.hInstance = hInstance;
    WinClass.lpszClassName = CLASS_NAME;

    RegisterClass(&WinClass);

    DWORD Style = WS_OVERLAPPEDWINDOW;
    HWND WindowHandle = CreateWindowEx(0, CLASS_NAME, L"Shura", Style,
                                       CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                                       NULL,
                                       NULL,
                                       hInstance,
                                       NULL);

    GlobalWin32WindowContext.Handle = WindowHandle;
    GlobalWin32WindowContext.ClearColor = CreateSolidBrush(RGB(48, 10, 36));

    ShowWindow(WindowHandle, CmdShow);

    platform_input_state InputState[2] = {};
    platform_input_state *OldInputState = InputState;
    platform_input_state *NewInputState = InputState + 1;

    renderer_context RendererContext = {};
    // TODO)): Get the AppName from the game dll.
    InitializeRenderer(&RendererContext, "Temporary Placeholder App Name");

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
    }

    DestroyRenderer(&RendererContext);
    // CloseWindow(WindowHandle);

    return 0;
}