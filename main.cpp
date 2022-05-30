
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define _UNICODE
#define UNICODE
#include <windows.h>

#include <commctrl.h>

#include "resource.h"

class MainWindow
{
    public:

        static LRESULT CALLBACK WindowProc(
                HWND hwnd,
                UINT uMsg,
                WPARAM wParam,
                LPARAM lParam);

        static DWORD WINAPI ClickerThreadProc(
                LPVOID lpParameter);

        MainWindow() : m_hwnd(NULL) {}

        HWND m_hwnd;

        BOOL Create(void);


        int m_interval_ms;
        bool m_clickerActive;

        UINT m_ClickerKey = VK_RBUTTON;

    private:

        LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

        int OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
        int OnCommand(HWND hwnd, long notificationCode, long identifier, HWND hwndControl);


        HINSTANCE m_hInstance;

        HWND m_hwndButtonStart;
        HWND m_hwndButtonEnd;

        HWND m_hwndDropDownClickerKey;

        HWND m_hwndEditInterval;
        HWND m_hwndUpdownInterval;

        HANDLE m_HandleClickerThread = NULL;

        HFONT m_hFontText;


        TCHAR m_ClickerKeyDescriptions[CLICKERKEY_COUNT][256];
};



int WinMain(
        HINSTANCE,
        HINSTANCE,
        LPSTR,
        int)
{
    MainWindow main_window;

    if (!main_window.Create())
    {
        return -1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}




LRESULT CALLBACK MainWindow::WindowProc(
        HWND hwnd,
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    MainWindow* pThis;

    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;

        pThis = (MainWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

        pThis->m_hwnd = hwnd;
    }
    else
    {
        pThis = (MainWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis)
    {
        return pThis->HandleMessage(uMsg, wParam, lParam);
    }
    else
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

BOOL MainWindow::Create(void)
{
    WNDCLASS winc;

    winc.style         = CS_HREDRAW | CS_VREDRAW;
    winc.lpfnWndProc   = MainWindow::WindowProc;
    winc.cbClsExtra    = 0;
    winc.cbWndExtra    = 0;
    winc.hInstance     = GetModuleHandle(NULL);
    winc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    winc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    winc.lpszMenuName  = NULL;
    winc.lpszClassName = TEXT("MainWindow");

    if (GetClassInfo(GetModuleHandle(NULL), winc.lpszClassName, &winc))
    {
    }
    else
    {
        if (!RegisterClass(&winc))
        {
            return FALSE;
        }
    }

    m_hwnd = CreateWindow(
            TEXT("MainWindow"), TEXT("Autoclicker"),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            500,
            500,
            NULL, NULL, GetModuleHandle(NULL), this
            );

    return (m_hwnd ? TRUE : FALSE);
}





void click(int button = 0)
{
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;

    inputs[1].type = INPUT_MOUSE;

    switch (button)
    {
        case 0:
            inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
            inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;
            break;
        case 1:
            inputs[0].mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            inputs[1].mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            break;
        case 2:
            inputs[0].mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
            inputs[1].mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
            break;
    }

    SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
}

#define IDC_RUN  1
#define IDC_STOP 2
#define IDC_CLICKERKEY 3

LRESULT MainWindow::HandleMessage(
        UINT uMsg,
        WPARAM wParam,
        LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_CREATE:
            ShowWindow(m_hwnd, SW_SHOW);
            return OnCreate(m_hwnd, (LPCREATESTRUCT)lParam);

        case WM_COMMAND:
            return OnCommand(m_hwnd, (long)HIWORD(wParam), (long)LOWORD(wParam), (HWND)lParam);

        default:
            return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
}



DWORD WINAPI MainWindow::ClickerThreadProc(
        LPVOID lpParameter)
{
    MessageBox(NULL, TEXT("Created"), TEXT("Created"), MB_ICONINFORMATION);
    MainWindow* lpHostWindow = (MainWindow*)lpParameter;
    while (lpHostWindow->m_clickerActive)
    {
        if (GetAsyncKeyState(lpHostWindow->m_ClickerKey) < 0)
        {
            click();
        }
        Sleep(lpHostWindow->m_interval_ms);
    }
    MessageBox(NULL, TEXT("Stopped"), TEXT("Stopped"), MB_ICONINFORMATION);
    return 0;
}


#define DEFAULT_INTERVAL 10

int MainWindow::OnCreate(
        HWND /*hwnd*/, LPCREATESTRUCT /*lpCreateStruct*/)
{
    m_hInstance = GetModuleHandle(NULL);


    m_hwndButtonStart = CreateWindow(
            TEXT("BUTTON"), TEXT("Run"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10, 10, 50, 40,
            m_hwnd, (HMENU)IDC_RUN, m_hInstance, NULL
            );

    m_hwndButtonEnd = CreateWindow(
            TEXT("BUTTON"), TEXT("Stop"),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            70, 10, 50, 40,
            m_hwnd, (HMENU)IDC_STOP, m_hInstance, NULL
            );

    m_hwndDropDownClickerKey = CreateWindow(
            TEXT("COMBOBOX"), NULL,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
            10, 60, 200, 200,
            m_hwnd, (HMENU)IDC_CLICKERKEY, m_hInstance, NULL
            );


    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof(INITCOMMONCONTROLSEX);

    icce.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icce);

    icce.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icce);


    m_hwndEditInterval = CreateWindow(
                              WC_EDIT, NULL,
                              WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER
                              | ES_NUMBER | ES_LEFT,
                              10, 100, 100, 21,
                              m_hwnd, NULL, m_hInstance, NULL
                              );

    m_hwndUpdownInterval = CreateWindow(
                              UPDOWN_CLASS, NULL,
                              WS_CHILDWINDOW | WS_VISIBLE
                              | UDS_AUTOBUDDY | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK,
                              0, 0, 0, 0,
                              m_hwnd, NULL, m_hInstance, NULL
                              );

    SendMessage(m_hwndUpdownInterval, UDM_SETRANGE, 0, MAKELPARAM(10000, 1));
    SendMessage(m_hwndUpdownInterval, UDM_SETPOS  , 0, DEFAULT_INTERVAL);


    for (int i = 0; i < CLICKERKEY_COUNT; i++)
    {
        LoadString(m_hInstance, g_ClickerKeys[i], m_ClickerKeyDescriptions[i], 256);
        SendMessage(m_hwndDropDownClickerKey, CB_ADDSTRING, 0, (LPARAM)m_ClickerKeyDescriptions[i]);
    }

    SendMessage(m_hwndDropDownClickerKey, CB_SETCURSEL, 0, 0);


    m_hFontText = CreateFont(
            18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
            VARIABLE_PITCH | FF_MODERN, TEXT("Segoe UI"));

    SendMessage(m_hwndButtonStart       , WM_SETFONT, (WPARAM)m_hFontText, TRUE);
    SendMessage(m_hwndButtonEnd         , WM_SETFONT, (WPARAM)m_hFontText, TRUE);
    SendMessage(m_hwndDropDownClickerKey, WM_SETFONT, (WPARAM)m_hFontText, TRUE);
    SendMessage(m_hwndEditInterval      , WM_SETFONT, (WPARAM)m_hFontText, TRUE);



    m_interval_ms = (int)SendMessage(m_hwndUpdownInterval, UDM_GETPOS, 0, 0);


    EnableWindow(m_hwndButtonEnd, FALSE);

    return 0;
}


int MainWindow::OnCommand(
        HWND /*hwnd*/, long notificationCode, long identifier, HWND /*hwndControl*/)
{
    switch (identifier)
    {
        case IDC_RUN:
            m_clickerActive = true;
            m_interval_ms = (int)SendMessage(m_hwndUpdownInterval, UDM_GETPOS, 0, 0);
            m_HandleClickerThread = CreateThread(
                    NULL,
                    0,
                    MainWindow::ClickerThreadProc,
                    this,
                    0,
                    NULL
                    );
            EnableWindow(m_hwndButtonStart       , FALSE);
            EnableWindow(m_hwndDropDownClickerKey, FALSE);
            EnableWindow(m_hwndEditInterval      , FALSE);
            EnableWindow(m_hwndDropDownClickerKey, FALSE);
            EnableWindow(m_hwndButtonEnd         , TRUE);
            break;

        case IDC_STOP:
            m_clickerActive = false;
            EnableWindow(m_hwndButtonStart       , TRUE);
            EnableWindow(m_hwndDropDownClickerKey, TRUE);
            EnableWindow(m_hwndEditInterval      , TRUE);
            EnableWindow(m_hwndDropDownClickerKey, TRUE);
            EnableWindow(m_hwndButtonEnd         , FALSE);
            break;

        case IDC_CLICKERKEY:
            switch (notificationCode)
            {
                case CBN_SELCHANGE:
                    m_ClickerKey = g_ClickerKeys[SendMessage(m_hwndDropDownClickerKey, CB_GETCURSEL, 0, 0)];
                    break;
            }
            break;
    }
    return 0;
}


