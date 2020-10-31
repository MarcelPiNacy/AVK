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



extern int vulkan_on_window_resize();



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
        case IDM_RUN_ALL_SORTS:
            abort();
            break;
        case IDM_SELECTION_SORT:
            algorithm_thread::assign_body(selection_sort);
            break;
        case IDM_INSERTION_SORT:
            algorithm_thread::assign_body(insertion_sort);
            break;
        case IDM_BUBBLE_SORT:
            algorithm_thread::assign_body(bubble_sort);
            break;
        case IDM_STD_SORT_HEAP:
            algorithm_thread::assign_body(std_sort_heap);
            break;
        case IDM_STD_STABLE_SORT:
            algorithm_thread::assign_body(std_stable_sort);
            break;
        case IDM_STD_SORT:
            algorithm_thread::assign_body(std_sort);
            break;
        case IDM_GRAIL_SORT:
            algorithm_thread::assign_body(block_merge_grail_sort);
            break;
        case IDM_GRAIL_SORT_STATIC:
            algorithm_thread::assign_body(block_merge_grail_sort_static);
            break;
        case IDM_GRAIL_SORT_CPP:
            algorithm_thread::assign_body(block_merge_grail_sort_cpp);
            break;
        case IDM_GRAIL_SORT_CPP_STATIC:
            algorithm_thread::assign_body(block_merge_grail_sort_cpp_static);
            break;
        case IDM_ODD_EVEN_MERGE_SORT:
            algorithm_thread::assign_body(odd_even_merge_sort);
            break;
        case IDM_BITONIC_SORT:
            algorithm_thread::assign_body(bitonic_sort);
            break;
        case IDM_FOLD_SORT:
            algorithm_thread::assign_body(fold_sort);
            break;
        case IDM_STD_MERGE_SORT:
            algorithm_thread::assign_body(std_merge_sort);
            break;
        case IDM_STD_INPLACE_MERGE_SORT:
            algorithm_thread::assign_body(std_inplace_merge_sort);
            break;
        case IDM_MSD_RADIX_SORT:
            algorithm_thread::assign_body(msd_radix_sort);
            break;
        case IDM_LSD_RADIX_SORT:
            algorithm_thread::assign_body(lsd_radix_sort);
            break;
        case IDM_AMERICAN_FLAG_SORT:
            algorithm_thread::assign_body(american_flag_sort_256);
            break;
        case IDM_INITIALIZE_ALREADY_SORTED:
            if (algorithm_thread::is_idle())
            {
                algorithm_thread::assign_body([](main_array& unused)
                {
                    main_array::for_each([&](item& e, uint32_t position)
                    {
                        item tmp;
                        tmp.value = position;
                        e = tmp;
                        e.original_position = position;
                        e.color = item_color::white();
                    });
                });
            }
            break;
        case IDM_INITIALIZE_REVERSED:
            if (algorithm_thread::is_idle())
            {
                algorithm_thread::assign_body([](main_array& unused)
                {
                    main_array::for_each([&](item& e, uint32_t position)
                    {
                        item tmp;
                        tmp.value = main_array::size() - position;
                        e = tmp;
                        e.original_position = position;
                        e.color = item_color::white();
                    });
                });
            }
            break;
        case IDM_INITIALIZE_ORGAN_PIPE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                algorithm_thread::assign_body([](main_array& unused)
                {
                    main_array::for_each([&](item& e, uint32_t position)
                    {
                        static uint32_t k = 0;
                        item tmp;
                        tmp.value = k;
                        if (position < main_array::size() / 2)
                            k += 2;
                        else
                            k -= 2;
                        e = tmp;
                        e.original_position = position;
                        e.color = item_color::white();
                    });
                });
            }
            break;
        case IDM_INITIALIZE_SHUFFLED:
            if (algorithm_thread::is_idle())
            {
                algorithm_thread::assign_body([](main_array& unused)
                {
                    struct romu_duo_functor
                    {
                        using result_type = uint64_t;

                        static constexpr auto min() { return 0; }
                        static constexpr auto max() { return UINT64_MAX; }

                        inline auto operator()() noexcept
                        {
                            return romu_duo_get();
                        }
                    };

                    romu_duo_set_seed(main_array::size() ^ time(nullptr));
                    romu_duo_functor tmp;
                    std::shuffle(main_array::begin(), main_array::end(), tmp);
                    main_array::for_each([](item& e, uint32_t position)
                    {
                        e.original_position = position;
                        e.color = item_color::white();
                    });
                });
            }
            break;
        case IDM_INITIALIZE_ROMUDUOJR:
            if (algorithm_thread::is_idle())
            {
                algorithm_thread::assign_body([](main_array& unused)
                {
                    romu_duo_set_seed(main_array::size() ^ time(nullptr));
                    main_array::for_each([](item& e, uint32_t position)
                    {
                        item tmp;
                        tmp.value = romu_duo_get() % main_array::size();
                        e = tmp;
                        e.original_position = position;
                        e.color = item_color::white();
                    });
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
        if (vulkan_on_window_resize() != 0)
            abort();
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