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
extern TCHAR window_title_buffer[4096];

extern int vulkan_on_window_resize();

static wchar_t dialog_box_buffer[4096 / sizeof(wchar_t)];

enum class array_mode
{
    LINEAR,
    LINEAR_REVERSE,
    ORGAN_PIPE_LINEAR,
    ORGAN_PIPE_LINEAR_REVERSE,
    RANDOM_SHUFFLE,
    RANDOM_ROMUDUOJR,
    QSORT_KILLER,
    MIN_HEAP,
    MAX_HEAP,
};

static array_mode last_array_mode = array_mode::LINEAR;

template <array_mode>
void modify_array();

template <>
void modify_array<array_mode::LINEAR>()
{
    main_array::for_each([&](item& e, uint32_t position)
    {
        item tmp;
        tmp.value = position;
        e = tmp;
        e.original_position = position;
        e.color = item_color::white();
    });
}

template <>
void modify_array<array_mode::LINEAR_REVERSE>()
{
    main_array::for_each([&](item& e, uint32_t position)
    {
        item tmp;
        tmp.value = main_array::size() - position;
        e = tmp;
        e.original_position = position;
        e.color = item_color::white();
    });
}

template <>
void modify_array<array_mode::ORGAN_PIPE_LINEAR>()
{
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

template <>
void modify_array<array_mode::ORGAN_PIPE_LINEAR_REVERSE>()
{
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

template <>
void modify_array<array_mode::QSORT_KILLER>()
{
    const uint k = main_array::size();
    const uint middle = k / 2;
    uint left = 0;
    uint right = middle;
    uint step = 2;
    uint staircase = 0;

    for (uint i = 0; i < k; ++i)
    {
        if ((i & 1) == 0)
        {
            item& e = main_array::get(left);
            e = {};
            e.color = item_color::white();
            e.value = i;
            e.original_position = left;

            left += step;
            if (left >= middle)
            {
                ++staircase;
                left = 1 << (uint8_t)_tzcnt_u64(staircase);
                --left;
                step *= 2;
            }
        }
        else
        {
            item& e = main_array::get(right);
            e = {};
            e.color = item_color::white();
            e.value = i;
            e.original_position = right;
            ++right;
        }
    }
    swap(main_array::get(middle - 1), main_array::get(k - 1));
}

template <>
void modify_array<array_mode::RANDOM_SHUFFLE>()
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
}

template <>
void modify_array<array_mode::RANDOM_ROMUDUOJR>()
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
}

template <>
void modify_array<array_mode::MIN_HEAP>()
{
    std::make_heap(main_array::begin(), main_array::end(), std::greater<>());
}

template <>
void modify_array<array_mode::MAX_HEAP>()
{
    std::make_heap(main_array::begin(), main_array::end(), std::less<>());
}

static void restore_last_distribution()
{
    switch (last_array_mode)
    {
    case array_mode::LINEAR:
        modify_array<array_mode::LINEAR>();
        break;
    case array_mode::LINEAR_REVERSE:
        modify_array<array_mode::LINEAR>();
        break;
    case array_mode::ORGAN_PIPE_LINEAR:
        modify_array<array_mode::LINEAR>();
        break;
    case array_mode::RANDOM_SHUFFLE:
        modify_array<array_mode::LINEAR>();
        break;
    case array_mode::RANDOM_ROMUDUOJR:
        modify_array<array_mode::LINEAR>();
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
                    sort_config::radix_size = (uint32_t)k;
                    if (__popcnt64(k) != 1)
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
                    sort_config::grail_sort_buffer_size = (uint32_t)k;
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
                    constexpr TCHAR title_format[] = TEXT("AVK - Sorting Algorithm Visualizer - [ %u elements ]");
#ifdef UNICODE
                    wsprintf(window_title_buffer, title_format, main_array::size());
                    SetWindowText(hwnd, window_title_buffer);
#else
                    sprintf(window_title_buffer, title_format, main_array::size());
                    SetWindowTextA(hwnd, window_title_buffer);
#endif
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
                    restore_last_distribution();
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
            algorithm_thread::assign_body(grail_sort);
            break;
        case IDM_ODD_EVEN_MERGE_SORT:
            algorithm_thread::assign_body(odd_even_merge_sort);
            break;
        case IDM_BITONIC_SORT:
            algorithm_thread::assign_body(bitonic_sort);
            break;
        case IDM_FOLD_SORT_TOP_DOWN:
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
        case IDM_MONITOR_SORT:
            algorithm_thread::assign_body(block_merge_monitor_sort);
            break;
        case IDM_WIKI_SORT:
            algorithm_thread::assign_body(wiki_sort);
            break;
        case IDM_GAMBIT_INSERTION_SORT:
            algorithm_thread::assign_body(gambit_insertion_sort);
            break;
        case IDM_STACKLESS_QUICK_SORT:
            algorithm_thread::assign_body(stackless_quick_sort);
            break;
        case IDM_FOLD_SORT_BOTTOM_UP:
            algorithm_thread::assign_body(fold_sort_bottom_up);
            break;
        case IDM_CUSTOM_RADIX_SORT:
            algorithm_thread::assign_body(custom_radix_sort);
            break;
        case IDM_SKA_SORT:
            algorithm_thread::assign_body(ska_sort);
            break;
        case IDM_SKA_SORT_COPY:
            algorithm_thread::assign_body(ska_sort_copy);
            break;
        case IDM_SQRT_SORT:
            algorithm_thread::assign_body(sqrt_sort);
            break;
        case IDM_BINARY_MSD_RADIX_SORT:
            algorithm_thread::assign_body(binary_msd_radix_sort);
            break;
        case IDM_INITIALIZE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::LINEAR;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::LINEAR>(); });
            }
            break;
        case IDM_INITIALIZE_REVERSE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::LINEAR_REVERSE;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::LINEAR_REVERSE>(); });
            }
            break;
        case IDM_INITIALIZE_ORGAN_PIPE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::ORGAN_PIPE_LINEAR;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::ORGAN_PIPE_LINEAR>(); });
            }
            break;
        case IDM_INITIALIZE_RANDOM_SHUFFLE:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::RANDOM_SHUFFLE;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::RANDOM_SHUFFLE>(); });
            }
            break;
        case IDM_INITIALIZE_RANDOM_ROMU_DUO_JR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::RANDOM_ROMUDUOJR;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::RANDOM_ROMUDUOJR>(); });
            }
            break;
        case IDM_INITIALIZE_REVERSE_ORGAN_PIPE_LINEAR:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::ORGAN_PIPE_LINEAR_REVERSE;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::ORGAN_PIPE_LINEAR_REVERSE>(); });
            }
            break;
        case IDM_QSORT_KILLER:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::QSORT_KILLER;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::QSORT_KILLER>(); });
            }
            break;
        case IDM_INITIALIZE_MAX_HEAP:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::MAX_HEAP;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::MAX_HEAP>(); });
            }
            break;
        case IDM_INITIALIZE_MIN_HEAP:
            if (algorithm_thread::is_idle())
            {
                last_array_mode = array_mode::MIN_HEAP;
                algorithm_thread::assign_body([](main_array& unused) { modify_array<array_mode::MIN_HEAP>(); });
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