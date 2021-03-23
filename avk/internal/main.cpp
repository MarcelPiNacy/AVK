#include "../external_dependencies/cmts/cmts.h"
#include "algorithm_thread.h"
#include "graphics/vulkan_state.h"
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

TCHAR window_title_buffer[4096];

extern LRESULT CALLBACK window_callbacks(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);



extern int init_vulkan();
extern void draw_main_array();



namespace cmts_checks
{
    const int count = 65536;

    alignas(64) std::atomic_int counter_a;
    alignas(64) std::atomic_int counter_b;

    static void count_test_task_a(void* unused)
    {
        counter_a.fetch_add(1, std::memory_order_relaxed);
    }

    static void count_test_task_b(void* unused)
    {
        counter_b.fetch_add(1, std::memory_order_relaxed);
    }

    static void count_test()
    {
        cmts_init_options_t options = {};
        options.task_stack_size = cmts_default_task_stack_size();
        options.max_tasks = count;
        options.thread_count = cmts_processor_count();
        auto code = cmts_lib_init(&options);
        assert(code == CMTS_OK);
        code = cmts_dispatch([](void* unused)
        {
            cmts_counter_t ca, cb;
            cmts_counter_init(&ca, count);
            cmts_counter_init(&cb, count);
            cmts_dispatch_options_t options = {};
            options.flags = CMTS_DISPATCH_FLAGS_FORCE;
            options.sync_type = CMTS_SYNC_TYPE_COUNTER;
            options.sync_object = &ca;
            for (int i = 0; i != count; ++i)
                cmts_dispatch(count_test_task_a, &options);
            options.sync_object = &cb;
            for (int i = 0; i != count; ++i)
                cmts_dispatch(count_test_task_b, &options);
            cmts_counter_await(&ca);
            cmts_counter_await(&cb);
            cmts_lib_exit_signal();
        }, nullptr);
        assert(code == CMTS_OK);
        code = cmts_lib_exit_await(nullptr);
        assert(code == CMTS_OK);
    }

    static void run()
    {
        count_test();
    }
}

static void update_title()
{
    char buffer[4096];
    
    uint64_t array_size = main_array::size();
    uint64_t comparissons = sort_stats::comparisson_count();
    uint64_t writes = sort_stats::write_count();
    sprintf_s(buffer, "AVK - Sorting Algorithm Visualizer - [ array size = %llu, comparissons = %llu, writes = %llu ]", array_size, comparissons, writes);
    SetWindowTextA(hwnd, buffer);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //cmts_checks::run();

    constexpr TCHAR class_name[] = TEXT("AVKClassName");

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

    constexpr TCHAR title[] = TEXT("AVK - Sorting Algorithm Visualizer");

    hwnd = CreateWindow(
        class_name,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 1920,
        CW_USEDEFAULT, 1080,
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
    
    auto delay = std::chrono::microseconds(5);
    main_array::set_compare_delay(delay);
    main_array::set_read_delay(delay);
    main_array::set_write_delay(delay);
    main_array::resize(1 << 18);
    
    main_array::for_each([&](item& e, uint32_t position)
    {
        e.value = position;
        e.original_position = position;
        e.color = item_color::white();
    });

    constexpr auto framerrate = milliseconds(16);
    constexpr auto title_update_threshold = milliseconds(60);

    auto last_draw = high_resolution_clock::now();
    auto last_title_update = last_draw;

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
        if (now - last_draw > framerrate)
        {
            draw_main_array();
            last_draw = now;
        }

        if (now - last_title_update > title_update_threshold)
        {
            update_title();
            last_title_update = now;
        }
    }

    algorithm_thread::terminate();
    return (int)msg.message;
}