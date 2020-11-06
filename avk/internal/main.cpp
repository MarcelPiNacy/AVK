#include "algorithm_thread.h"
#include "windows-specific/framework.h"
#include "windows-specific/Resource.h"
#include <atomic>
#include <chrono>

#define MAX_LOADSTRING 100

HINSTANCE hinstance;
HWND hwnd;
static HACCEL accel;

std::atomic<bool> should_continue_global = true;
std::atomic<bool> should_continue_sort;

static TCHAR window_title_buffer[1 << 12];

extern LRESULT CALLBACK window_callbacks(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



extern int init_vulkan();
extern void draw_main_array();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    constexpr TCHAR class_name[] = TEXT("ArrayVKClassName");

    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = window_callbacks;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ARRAYVK));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateSolidBrush(0);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MAIN_MENU);
    wcex.lpszClassName = class_name;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    constexpr auto base_counter = __COUNTER__;

    if (RegisterClassEx(&wcex) == INVALID_ATOM)
        return -(__COUNTER__ - base_counter);

    hinstance = hInstance;

    constexpr TCHAR title[] = TEXT("ArrayVK - Sorting Algorithm Visualizer");

    hwnd = CreateWindow(
        class_name,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0,
        CW_USEDEFAULT, 0,
        nullptr, nullptr,
        hInstance, nullptr);

    if (hwnd == nullptr)
        return -(__COUNTER__ - base_counter);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAIN_MENU));

    const int res = init_vulkan();
    if (res < 0)
        return (1 << 30) | res;

    algorithm_thread::launch();

    using namespace std::chrono;

    auto last = high_resolution_clock::now();

    item_color color = item_color::white();
    double delay = 0.0001;
    main_array::set_compare_delay(delay);
    main_array::set_read_delay(delay);
    main_array::set_write_delay(delay);
    main_array::resize(1 << 11);
    
    constexpr TCHAR title_format[] = TEXT("AVK - Sorting Algorithm Visualizer - [ %u elements ]");

#ifdef UNICODE
    wsprintf(window_title_buffer, title_format, main_array::size());
    SetWindowText(hwnd, window_title_buffer);
#else
    sprintf(window_title_buffer, title_format, main_array::size());
    SetWindowTextA(hwnd, window_title_buffer);
#endif

    main_array::for_each([&](item& e, uint32_t position)
    {
        e.value = position;
        e.original_position = position;
        e.color = color;
    });

    constexpr auto MAX_FPS = milliseconds(8);

    MSG msg = {};
    while (should_continue_global.load(std::memory_order_acquire))
    {
        while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, accel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        draw_main_array();
    }

    algorithm_thread::terminate(); //die
    return (int)msg.message;
}