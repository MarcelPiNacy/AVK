#include "algorithm_thread.h"
#include "windows-specific/framework.h"
#include "windows-specific/Resource.h"
#include "../algorithms/all.h"
#include "prng.h"
#include <ctime>
#include <algorithm>



extern std::atomic<bool> should_continue_global;
extern HINSTANCE hinstance;
extern HWND hwnd;



extern void vulkan_on_window_resize();



INT_PTR CALLBACK about_callbacks(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}



LRESULT CALLBACK window_callbacks(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    item_color color = item_color::white();

    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_CREDITS:
            DialogBox(hinstance, MAKEINTRESOURCE(IDD_CREDITSBOX), hWnd, about_callbacks);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_SELECTION_SORT:
            algorithm_thread::assign_sort(selection_sort);
            break;
        case IDM_INSERTION_SORT:
            algorithm_thread::assign_sort(insertion_sort);
            break;
        case IDM_BUBBLE_SORT:
            algorithm_thread::assign_sort(bubble_sort);
            break;
        case IDM_STD_SORT_HEAP:
            algorithm_thread::assign_sort(std_sort_heap);
            break;
        case IDM_STD_STABLE_SORT:
            algorithm_thread::assign_sort(std_stable_sort);
            break;
        case IDM_STD_SORT:
            algorithm_thread::assign_sort(std_sort);
            break;
        case IDM_GRAIL_SORT:
            algorithm_thread::assign_sort(block_merge_grail_sort);
            break;
        case IDM_FOLD_SORT:
            algorithm_thread::assign_sort(fold_sort);
            break;
        case IDM_BITONIC_SORT:
            algorithm_thread::assign_sort(bitonic_sort);
            break;
        case IDM_INITIALIZE_ALREADY_SORTED:
            if (algorithm_thread::is_idle())
            {
                main_array::fill([&](item& e, uint32_t position)
                {
                    e.value = position;
                    e.original_position = position;
                    e.color = color;
                    //color.r += 1.0f / (float)main_array::size();
                });
            }
            break;
        case IDM_INITIALIZE_REVERSED:
            if (algorithm_thread::is_idle())
            {
                main_array::fill([&](item& e, uint32_t position)
                {
                    e.value = main_array::size() - position;
                    e.original_position = position;
                    e.color = color;
                    //color.r += 1.0f / (float)main_array::size();
                });
            }
            break;
        case IDM_INITIALIZE_ORGAN_PIPE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                uint32_t k = 0;
                bool forward = true;
                const auto mid = main_array::size() / 2;
                main_array::fill([&](item& e, uint32_t position)
                {
                    e.value = k;
                    if (position > mid)
                        forward = false;
                    if (forward)
                        k += 2;
                    else
                        k -= 2;
                    e.original_position = position;
                    e.color = color;
                });
            }
            break;
        case IDM_INITIALIZE_SHUFFLED:
            if (algorithm_thread::is_idle())
            {
                romu_duo_set_seed(main_array::size() ^ time(nullptr));
                romu_duo_functor tmp;
                std::shuffle(main_array::begin(), main_array::end(), tmp);
            }
            break;
        case IDM_INITIALIZE_ROMUDUOJR:
            if (algorithm_thread::is_idle())
            {
                romu_duo_set_seed(main_array::size() ^ time(nullptr));
                main_array::fill([&](item& e, uint32_t position)
                {
                    e.value = romu_duo_get() % main_array::size();
                    e.original_position = position;
                    e.color = color;
                });
            }
            break;
        case IDM_PAUSE_SIMULATION:
            algorithm_thread::pause();
            break;
        case IDM_RESUME_SIMULATION:
            algorithm_thread::resume();
            break;
        case IDM_ABORT_SIMULATION:
            algorithm_thread::abort_sort();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_SIZE:
        vulkan_on_window_resize();
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        should_continue_global.store(false, std::memory_order_release);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}