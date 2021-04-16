#include "all.h"
#include "../internal/parallel_for.h"

static void odd_even_merge_sort_kernel(main_array array, size_t lo, size_t m2, size_t n, size_t r)
{
    size_t m = r * 2;
    if (m < n)
    {
        if ((n / r) % 2 != 0)
        {
            size_t params[2][3] =
            {
                { lo, (m2 + 1) / 2, n + r },
                { lo + r, m2 / 2, n - r }
            };

            parallel_for<size_t>(0, 2, [=](size_t i)
            {
                odd_even_merge_sort_kernel(array, params[i][0], params[i][1], params[i][2], m);
            });
        }
        else
        {
            size_t params[2][3] =
            {
                { lo, (m2 + 1) / 2, n },
                { lo + r, m2 / 2, n }
            };

            parallel_for<size_t>(0, 2, [=](size_t i)
            {
                odd_even_merge_sort_kernel(array, params[i][0], params[i][1], params[i][2], m);
            });
        }

        if (m2 % 2 != 0)
        {
            for (size_t i = lo; i + r < lo + n; i += m)
            {
                compare_swap(array, i, i + r);
            }
        }
        else
        {
            for (size_t i = lo + r; i + r < lo + n; i += m)
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

static void odd_even_merge_sort_core(main_array array, size_t lo, size_t n)
{
    if (n > 1)
    {
        size_t m = n / 2;
        size_t params[2][2] =
        {
            { lo, m },
            { lo + m, n - m }
        };
        
        parallel_for<size_t>(0, 2, [=](size_t i)
        {
            odd_even_merge_sort_core(array, params[i][0], params[i][1]);
        });

        odd_even_merge_sort_kernel(array, lo, m, n, 1);
    }
}


void odd_even_merge_sort_parallel(main_array array)
{
    odd_even_merge_sort_core(array, 0, array.size());
}