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
    wcex.hbrBackground = CreateSolidBrush(0);
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

    const int res = init_vulkan();
    if (res < 0)
        return (1 << 30) | res;

    algorithm_thread::launch();

    using namespace std::chrono;

    auto last = high_resolution_clock::now();

    item_color color = { 1, 0, 0 };
    double delay = 0.01;
    main_array::set_compare_delay(delay);
    main_array::set_read_delay(delay);
    main_array::set_write_delay(delay);
    main_array::resize(1 << 8);
    main_array::fill([&](item& e, uint32_t position)
    {
        e.value = main_array::size() - position;
        e.original_position = position;
        e.color = color;
        color.r -= 1.0f / (float)main_array::size();
    });

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
        if (duration_cast<milliseconds>(now - last).count() >= 16)
        {
            draw_main_array();
            last = now;
        }
    }

    algorithm_thread::terminate(); //die
    return (int)msg.message;
}