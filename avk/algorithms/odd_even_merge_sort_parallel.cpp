#include "all.h"
#include <thread>

using std::thread;

static void odd_even_merge_sort_kernel(main_array array, int lo, int m2, int n, int r)
{
    int m = r * 2;
    if (m < n)
    {
        thread threads[2] = {};
        if ((n / r) % 2 != 0)
        {
            threads[0] = thread(odd_even_merge_sort_kernel, array, lo, (m2 + 1) / 2, n + r, m);
            threads[1] = thread(odd_even_merge_sort_kernel, array, lo + r, m2 / 2, n - r, m);
        }
        else
        {
            threads[0] = thread(odd_even_merge_sort_kernel, array, lo, (m2 + 1) / 2, n, m);
            threads[1] = thread(odd_even_merge_sort_kernel, array, lo + r, m2 / 2, n, m);
        }

        for (thread& t : threads)
            t.join();

        if (m2 % 2 != 0)
        {
            for (int i = lo; i + r < lo + n; i += m)
            {
                compare_swap(array, i, i + r);
            }
        }
        else
        {
            for (int i = lo + r; i + r < lo + n; i += m)
            {
                compare_swap(array, i, i + r);
            }
        }
    }
    else
    {
        if (n > r)
        {
            compare_swap(array, lo, lo + r);
        }
    }
}

static void odd_even_merge_sort_core(main_array array, int lo, int n)
{
    if (n > 1)
    {
        int m = n / 2;
        thread threads[2] = {};
        threads[0] = thread(odd_even_merge_sort_core, array, lo, m);
        threads[1] = thread(odd_even_merge_sort_core, array, lo + m, n - m);
        for (thread& t : threads)
            t.join();
        odd_even_merge_sort_kernel(array, lo, m, n, 1);
    }
}


void odd_even_merge_sort_parallel(main_array array)
{
    run_as_parallel([=]() { odd_even_merge_sort_core(array, 0, array.size()); });
}