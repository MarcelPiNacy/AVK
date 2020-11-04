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

static wchar_t dialog_box_buffer[4096 / sizeof(wchar_t)];

enum class array_mode
{
    LINEAR,
    REVERSE,
    ORGAN_PIPE_LINEAR,
    REVERSE_ORGAN_PIPE_LINEAR,
    SHUFFLED,
    RANDOM_ROMUDUOJR,
};

static array_mode last_array_mode = array_mode::LINEAR;

static void array_lineal_init()
{
    last_array_mode = array_mode::LINEAR;
    main_array::for_each([&](item& e, uint32_t position)
    {
        item tmp;
        tmp.value = position;
        e = tmp;
        e.original_position = position;
        e.color = item_color::white();
    });
}

static void array_reverse_init()
{
    last_array_mode = array_mode::REVERSE;
    main_array::for_each([&](item& e, uint32_t position)
    {
        item tmp;
        tmp.value = main_array::size() - position;
        e = tmp;
        e.original_position = position;
        e.color = item_color::white();
    });
}

static void array_organ_pipe_lineal_init()
{
    last_array_mode = array_mode::ORGAN_PIPE_LINEAR;
    uint32_t k = 0;
    main_array::for_each([&](item& e, uint32_t position)
    {
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
}

static void array_reverse_organ_pipe_lineal_init()
{
    last_array_mode = array_mode::ORGAN_PIPE_LINEAR;
    uint32_t k = main_array::size();
    main_array::for_each([&](item& e, uint32_t position)
    {
        item tmp;
        tmp.value = k;
        if (position < main_array::size() / 2)
            k -= 2;
        else
            k += 2;
        e = tmp;
        e.original_position = position;
        e.color = item_color::white();
    });
}

static void array_shuffled_init()
{
    last_array_mode = array_mode::SHUFFLED;
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
}

static void array_random_romuduojr_init()
{
    last_array_mode = array_mode::RANDOM_ROMUDUOJR;
    romu_duo_set_seed(main_array::size() ^ time(nullptr));
    main_array::for_each([](item& e, uint32_t position)
    {
        item tmp;
        tmp.value = romu_duo_get() % main_array::size();
        e = tmp;
        e.original_position = position;
        e.color = item_color::white();
    });
}

static void restore_last_distribution()
{
    switch (last_array_mode)
    {
    case array_mode::LINEAR:
        array_lineal_init();
        break;
    case array_mode::REVERSE:
        array_reverse_init();
        break;
    case array_mode::ORGAN_PIPE_LINEAR:
        array_organ_pipe_lineal_init();
        break;
    case array_mode::SHUFFLED:
        array_shuffled_init();
        break;
    case array_mode::RANDOM_ROMUDUOJR:
        array_random_romuduojr_init();
        break;
    default:
        break;
    }
}

static INT_PTR CALLBACK about_callbacks(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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

static INT_PTR CALLBACK set_radix_size_callbacks(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (GetDlgItemText(hDlg, IDD_SET_RADIX_SIZE_TEXTBOX, dialog_box_buffer, sizeof(dialog_box_buffer) / 2))
            {
                uint64_t k = 0;
                if (swscanf_s(dialog_box_buffer, L"%llu", &k) == 1)
                {
                    sort_config::radix_size = k;
                    if (__popcnt(k) != 1)
                    {
                        MessageBox(
                            nullptr,
                            L"Error: Non power of 2 inputs are illegal.",
                            L"Invalid Radix!",
                            MB_OK | MB_ICONERROR);
                    }
                }
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)FALSE;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static INT_PTR CALLBACK set_grailsort_buffer_size_callbacks(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (GetDlgItemText(hDlg, IDD_SET_GRAILSORT_BUFFER_SIZE_TEXTBOX, dialog_box_buffer, sizeof(dialog_box_buffer) / 2))
            {
                uint64_t k = 0;
                if (swscanf_s(dialog_box_buffer, L"%llu", &k) == 1)
                    sort_config::grail_sort_buffer_size = k;
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        default:
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

static INT_PTR CALLBACK set_array_size_callbacks(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
            if (GetDlgItemText(hDlg, IDD_SET_ARRAY_SIZE_TEXTBOX, dialog_box_buffer, sizeof(dialog_box_buffer) / 2))
            {
                uint32_t k = 0;
                if (swscanf_s(dialog_box_buffer, L"%u", &k) == 1)
                {
                    if (!main_array::resize(k))
                        return (INT_PTR)FALSE;

                }
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            break;
        default:
            break;
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
            algorithm_thread::assign_body([](main_array& array)
            {
                for (auto fn : sort_table)
                {
                    switch (last_array_mode)
                    {
                    case array_mode::LINEAR:
                        array_lineal_init();
                        break;
                    case array_mode::REVERSE:
                        array_reverse_init();
                        break;
                    case array_mode::ORGAN_PIPE_LINEAR:
                        array_organ_pipe_lineal_init();
                        break;
                    case array_mode::REVERSE_ORGAN_PIPE_LINEAR:
                        array_reverse_organ_pipe_lineal_init();
                        break;
                    case array_mode::SHUFFLED:
                        array_shuffled_init();
                        break;
                    case array_mode::RANDOM_ROMUDUOJR:
                        array_random_romuduojr_init();
                        break;
                    default:
                        break;
                    }
                    fn(array);
                    main_array::sleep(1);
                }
            });
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
        case IDM_ODD_EVEN_SORT:
            algorithm_thread::assign_body(odd_even_sort);
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
        case IDM_GRAIL_SORT_CPP:
            algorithm_thread::assign_body(block_merge_grail_sort_cpp);
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
            algorithm_thread::assign_body(american_flag_sort);
            break;
        case IDM_RADIX_TREE_SORT:
            algorithm_thread::assign_body(radix_tree_sort);
            break;
        case IDM_COUNTING_SORT:
            algorithm_thread::assign_body(radix_tree_sort);
            break;
        case IDM_BINARY_TREE_SORT:
            algorithm_thread::assign_body(binary_tree_sort);
            break;
        case IDM_INITIALIZE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::LINEAR;
                algorithm_thread::assign_body([](main_array& unused) { array_lineal_init(); });
            }
            break;
        case IDM_INITIALIZE_REVERSE:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::REVERSE;
                algorithm_thread::assign_body([](main_array& unused) { array_reverse_init(); });
            }
            break;
        case IDM_INITIALIZE_ORGAN_PIPE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::ORGAN_PIPE_LINEAR;
                algorithm_thread::assign_body([](main_array& unused) { array_organ_pipe_lineal_init(); });
            }
            break;
        case IDM_INITIALIZE_SHUFFLED:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::SHUFFLED;
                algorithm_thread::assign_body([](main_array& unused) { array_shuffled_init(); });
            }
            break;
        case IDM_INITIALIZE_ROMUDUOJR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::RANDOM_ROMUDUOJR;
                algorithm_thread::assign_body([](main_array& unused) { array_random_romuduojr_init(); });
            }
            break;
        case IDM_INITIALIZE_REVERSE_ORGAN_PIPE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::REVERSE_ORGAN_PIPE_LINEAR;
                algorithm_thread::assign_body([](main_array& unused) { array_reverse_organ_pipe_lineal_init(); });
            }
            break;
        case IDM_SET_RADIX_SIZE:
            DialogBox(hinstance, MAKEINTRESOURCE(IDD_SET_RADIX_SIZE_BOX), hWnd, set_radix_size_callbacks);
            break;
        case IDM_SET_GRAILSORT_BUFFER_SIZE:
            DialogBox(hinstance, MAKEINTRESOURCE(IDD_SET_GRAILSORT_BUFFER_SIZE_BOX), hWnd, set_grailsort_buffer_size_callbacks);
            break;
        case IDM_SET_ARRAY_SIZE:
            DialogBox(hinstance, MAKEINTRESOURCE(IDD_SET_ARRAY_SIZE_BOX), hWnd, set_array_size_callbacks);
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
        {
            abort();
        }
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