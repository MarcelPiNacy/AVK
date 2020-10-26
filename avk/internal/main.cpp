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
    wcex.hbrBackground = CreateSolidBrush(0x0d0c00);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ARRAYVK);
    wcex.lpszClassName = class_name;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    constexpr auto base_counter = __COUNTER__;

    if (RegisterClassEx(&wcex) == INVALID_ATOM)
        return -(__COUNTER__ - base_counter);

    hinstance = hInstance;

    const TCHAR title[] = TEXT("ArrayVK - Sorting Algorithm Visualizer");

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

    accel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ARRAYVK));

    if (init_vulkan() < 0)
        return -(__COUNTER__ - base_counter);

    algorithm_thread::launch();

    using namespace std::chrono;

    constexpr auto framerrate = milliseconds(120);
    auto last = high_resolution_clock::now();

    main_array::resize(1 << 10);
    main_array::fill([](element& e, uint32_t position)
    {
        e.internal_value = position;
        e.initial_position = position;
    });
    main_array::set_read_delay(1);
    main_array::set_write_delay(1);
    main_array::set_compare_delay(1);

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

        const auto now = high_resolution_clock::now();
        if (duration_cast<milliseconds>(now - last) >= framerrate)
        {
            draw_main_array();
            last = now;
        }
    }

    return (int)msg.message;
}