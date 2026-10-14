#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#define WM_TRAY (WM_APP + 1)
#define ID_TOGGLE 1
#define ID_EXIT 2
#define ID_TIMER 1
#define ICON_SIZE 32

static NOTIFYICONDATAW nid;
static HICON iconActive, iconPaused;
static BOOL paused = FALSE;

static void ApplyState(void)
{
    if (!paused)
        SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
}

/* Draw a filled circle with a white ring. color is 0xRRGGBB. */
static HICON MakeDotIcon(DWORD color)
{
    BITMAPINFO bi = {0};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = ICON_SIZE;
    bi.bmiHeader.biHeight = -ICON_SIZE;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    DWORD *px = NULL;
    HDC dc = GetDC(NULL);
    HBITMAP colorBmp = CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void **)&px, NULL, 0);
    ReleaseDC(NULL, dc);
    HBITMAP maskBmp = CreateBitmap(ICON_SIZE, ICON_SIZE, 1, 1, NULL);

    const int c = ICON_SIZE / 2;
    const int rOuter = 14, rInner = 12;
    for (int y = 0; y < ICON_SIZE; y++)
    {
        for (int x = 0; x < ICON_SIZE; x++)
        {
            int dx = x - c, dy = y - c;
            int d2 = dx * dx + dy * dy;
            if (d2 <= rInner * rInner)
                px[y * ICON_SIZE + x] = 0xFF000000 | color;
            else if (d2 <= rOuter * rOuter)
                px[y * ICON_SIZE + x] = 0xFFFFFFFF;
            else
                px[y * ICON_SIZE + x] = 0;
        }
    }

    ICONINFO ii = {0};
    ii.fIcon = TRUE;
    ii.hbmColor = colorBmp;
    ii.hbmMask = maskBmp;
    HICON icon = CreateIconIndirect(&ii);
    DeleteObject(colorBmp);
    DeleteObject(maskBmp);
    return icon;
}

static void UpdateTray(void)
{
    nid.hIcon = paused ? iconPaused : iconActive;
    lstrcpyW(nid.szTip, paused ? L"ZenJiggle \x2014 Paused" : L"ZenJiggle \x2014 Active");
    nid.uFlags = NIF_ICON | NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

static void TogglePause(void)
{
    paused = !paused;
    if (paused)
        SetThreadExecutionState(ES_CONTINUOUS); /* give control back to Windows */
    else
        ApplyState();
    UpdateTray();
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_TIMER:
        ApplyState();
        return 0;

    case WM_TRAY:
        if (lp == WM_LBUTTONDBLCLK)
        {
            TogglePause();
        }
        else if (lp == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            HMENU menu = CreatePopupMenu();
            AppendMenuW(menu, MF_STRING, ID_TOGGLE, paused ? L"Resume" : L"Pause");
            AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(menu, MF_STRING, ID_EXIT, L"Exit");
            SetForegroundWindow(hwnd); /* needed so the menu closes on outside click */
            TrackPopupMenu(menu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
            PostMessageW(hwnd, WM_NULL, 0, 0);
            DestroyMenu(menu);
        }
        return 0;

    case WM_COMMAND:
        if (LOWORD(wp) == ID_TOGGLE)
            TogglePause();
        else if (LOWORD(wp) == ID_EXIT)
            DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        SetThreadExecutionState(ES_CONTINUOUS);
        KillTimer(hwnd, ID_TIMER);
        Shell_NotifyIconW(NIM_DELETE, &nid);
        DestroyIcon(iconActive);
        DestroyIcon(iconPaused);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

/* Entry point without the C runtime, to keep the exe tiny. */
void WINAPI WinMainCRTStartup(void)
{
    /* Only one copy at a time. */
    HANDLE mutex = CreateMutexW(NULL, TRUE, L"ZenJiggle.SingleInstance.Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        ExitProcess(0);

    HINSTANCE inst = GetModuleHandleW(NULL);
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.lpszClassName = L"ZenJiggleWnd";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"ZenJiggle", 0, 0, 0, 0, 0,
                                HWND_MESSAGE, NULL, inst, NULL);

    iconActive = MakeDotIcon(0x2ECC71); /* green */
    iconPaused = MakeDotIcon(0xE74C3C); /* red */

    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAY;
    nid.hIcon = iconActive;
    lstrcpyW(nid.szTip, L"ZenJiggle \x2014 Active");
    Shell_NotifyIconW(NIM_ADD, &nid);

    SetTimer(hwnd, ID_TIMER, 30000, NULL);
    ApplyState();

    MSG m;
    while (GetMessageW(&m, NULL, 0, 0) > 0)
    {
        TranslateMessage(&m);
        DispatchMessageW(&m);
    }

    ReleaseMutex(mutex);
    ExitProcess(0);
}
