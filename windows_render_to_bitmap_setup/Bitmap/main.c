#include <Windows.h>
#ifdef _DEBUG
#include "assert.h"
#endif

#include "win32_logging.h"
#include <stdlib.h>
#include <stdint.h>

#include "pawel_renderer.h"

static const size_t BITMAP_WIDTH = 960;
static const size_t BITMAP_HEIGHT = 540;



static DWORD g_main_thread_id;


#define WIN32_EVENT_TO_HIDDEN_WINDOW_CREATE_REAL_WINDOW (WM_USER + 1)
#define WIN32_EVENT_TO_HIDDEN_WINDOW_DESTROY_REAL_WINDOW (WM_USER + 2)

#define WIN32_EVENT_TO_MAIN_THREAD_WINDOW_WAS_CREATED (WM_USER + 1)



static LRESULT CALLBACK mainWindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    switch (message) {
    case WM_CHAR:
        PostThreadMessageW(g_main_thread_id, message, wParam, lParam);
        return 0;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
}




static DWORD WINAPI mainThread(LPVOID param) {
    HWND hidden_window = (HWND)param;
    MSG main_thread_message = {0};
    ATOM real_window_class;
    WNDCLASSEXW real_window_class_desc = { 0 };
    HWND real_window = 0;

    BITMAPINFO bitmap_info = { 0 };
    uint32_t *bitmap;
    unsigned char is_running = 1;
    HDC device_context;
    RECT real_window_rect;
    int real_window_width, real_window_height;



    PeekMessageW(&main_thread_message, NULL, 0, 0, PM_NOREMOVE); // This creates queue

    
    real_window_class_desc.cbSize = sizeof(real_window_class_desc);
    real_window_class_desc.style = CS_HREDRAW | CS_VREDRAW;
    real_window_class_desc.lpfnWndProc = mainWindowProcedure;
    real_window_class_desc.cbClsExtra = 0;
    real_window_class_desc.cbWndExtra = 0;
    real_window_class_desc.hInstance = GetModuleHandleW(NULL);;
    real_window_class_desc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    real_window_class_desc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    real_window_class_desc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    real_window_class_desc.lpszMenuName = NULL;
    real_window_class_desc.lpszClassName = L"b";
    real_window_class_desc.hIconSm = NULL;

    real_window_class = RegisterClassExW(&real_window_class_desc);
    if (real_window_class == 0) {
        WIN32_LOG_LAST_ERROR(GetLastError(), "RegisterClassExW failed when creating real_window_class");
        ExitProcess(1);
    }
    
    SendMessageW(hidden_window, WIN32_EVENT_TO_HIDDEN_WINDOW_CREATE_REAL_WINDOW, real_window_class, 0);
    while (GetMessageW(&main_thread_message, NULL, 0, 0)) {
        if (main_thread_message.message == WIN32_EVENT_TO_MAIN_THREAD_WINDOW_WAS_CREATED) {
            real_window = (HWND)main_thread_message.wParam;
            break;
        }
    }
    if (real_window == NULL) {
        return 1;
    }
    bitmap = (uint32_t*)malloc(sizeof(*bitmap) * BITMAP_WIDTH * BITMAP_HEIGHT);

    bitmap_info.bmiHeader.biSize = sizeof(bitmap_info.bmiHeader);
    bitmap_info.bmiHeader.biWidth = BITMAP_WIDTH;
    bitmap_info.bmiHeader.biHeight = -1 * (LONG)BITMAP_HEIGHT;
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;

    device_context = GetDC(real_window);




    while (is_running) {
        while (PeekMessageW(&main_thread_message, NULL, 0, 0, PM_REMOVE)) {
            switch (main_thread_message.message) {
            case WM_CHAR:
                WIN32_LOG("Pacłeś klawisz");
                break;
            default:
#ifdef _DEBUG
                WIN32_LOG("Nie obsłużyłeś eventu: %u", main_thread_message.message);
                assert(0);
#endif
                break;
            }
        }
        renderToBitmap(bitmap, BITMAP_WIDTH, BITMAP_HEIGHT);

        GetClientRect(real_window, &real_window_rect);
        real_window_width = real_window_rect.right - real_window_rect.left;
        real_window_height = real_window_rect.bottom - real_window_rect.top;

        StretchDIBits(
            device_context,
            0, 0, real_window_width, real_window_height,
            0, 0, BITMAP_WIDTH, BITMAP_HEIGHT,
            bitmap, &bitmap_info,
            DIB_RGB_COLORS,
            SRCCOPY
        );

    }



    return 0;

}





static LRESULT CALLBACK hiddenWindowProcedure(
    HWND window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    ATOM real_window_class = (ATOM)wParam;
    HWND real_window;

    switch (message) {
    case WIN32_EVENT_TO_HIDDEN_WINDOW_CREATE_REAL_WINDOW:
        real_window_class = (ATOM)wParam;
        real_window = CreateWindowExW(
            0,
            MAKEINTATOM(real_window_class),
            L"Window",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            BITMAP_WIDTH,
            BITMAP_HEIGHT,
            NULL,
            NULL,
            GetModuleHandleW(NULL),
            NULL
        );
        if (real_window == NULL) {
            WIN32_LOG_LAST_ERROR(GetLastError(), "CreateWindowExW failed when creating real_window");
            ExitProcess(1);
        }
        ShowWindow(real_window, SW_SHOW);
        PostThreadMessageW(g_main_thread_id, WIN32_EVENT_TO_MAIN_THREAD_WINDOW_WAS_CREATED, real_window, NULL);

        return 0;
        break;
    default:
        return DefWindowProcW(window, message, wParam, lParam);
    }
    
}



int WinMain(
	_In_ HINSTANCE instance,
	_In_opt_ HINSTANCE prev_instance,
	_In_ LPSTR command_line,
	_In_ int show_command
) {
    win32LogInitialize();

    ATOM hidden_window_class;
    WNDCLASSEXW hidden_window_class_desc = { 0 };
    HWND hidden_window = { 0 };
    MSG hidden_window_message = { 0 };

    hidden_window_class_desc.cbSize = sizeof(hidden_window_class_desc);
    hidden_window_class_desc.style = 0;
    hidden_window_class_desc.lpfnWndProc = hiddenWindowProcedure;
    hidden_window_class_desc.cbClsExtra = 0;
    hidden_window_class_desc.cbWndExtra = 0;
    hidden_window_class_desc.hInstance = instance;
    hidden_window_class_desc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    hidden_window_class_desc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    hidden_window_class_desc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    hidden_window_class_desc.lpszMenuName = NULL;
    hidden_window_class_desc.lpszClassName = L"a";
    hidden_window_class_desc.hIconSm = NULL;
    
    hidden_window_class = RegisterClassExW(&hidden_window_class_desc);
    if (hidden_window_class == 0) {
        WIN32_LOG_LAST_ERROR(GetLastError(), "RegisterClassExW failed when creating hidden_window_class");
        return 1;
    }

    hidden_window = CreateWindowExW(
        0,
        MAKEINTATOM(hidden_window_class),
        L"HIDDEN WINDOW",
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        0,
        0,
        hidden_window_class_desc.hInstance,
        0
    );
    if (hidden_window == NULL) {
        WIN32_LOG_LAST_ERROR(GetLastError(), "CreateWindowExW failed when creating hidden_window");
        return 1;
    }

    CreateThread(NULL, 0, mainThread, hidden_window, 0, &g_main_thread_id);

    while (GetMessageW(&hidden_window_message, 0, 0, 0)) {
        TranslateMessage(&hidden_window_message);
        DispatchMessageW(&hidden_window_message);
    }

#ifdef _DEBUG
    win32LogDeInitialize();
#endif
	return 0;
}