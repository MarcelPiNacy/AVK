#include "all.h"

static void odd_even_merge_sort_kernel(main_array array, uint lo, uint m2, uint n, uint r)
{
    int m = r * 2;
    if (m < n)
    {
        if ((n / r) % 2 != 0)
        {
            uint params[2][3] =
            {
                { lo, (m2 + 1) / 2, n + r },
                { lo + r, m2 / 2, n - r }
            };

            parallel_for<uint>(0, 2, [=](uint i)
            {
                odd_even_merge_sort_kernel(array, params[i][0], params[i][1], params[i][2], m);
            });
        }
        else
        {
            uint params[2][3] =
            {
                { lo, (m2 + 1) / 2, n },
                { lo + r, m2 / 2, n }
            };

            parallel_for<uint>(0, 2, [=](uint i)
            {
                odd_even_merge_sort_kernel(array, params[i][0], params[i][1], params[i][2], m);
            });
        }

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

static void odd_even_merge_sort_core(main_array array, uint lo, uint n)
{
    if (n > 1)
    {
        int m = n / 2;
        uint params[2][2] =
        {
            { lo, m },
            { lo + m, n - m }
        };
        
        parallel_for<uint>(0, 2, [=](uint i)
        {
            odd_even_merge_sort_core(array, params[i][0], params[i][1]);
        });

        odd_even_merge_sort_kernel(array, lo, m, n, 1);
    }
}


void odd_even_merge_sort_parallel(main_array array)
{
    as_parallel([=]()
    {
        odd_even_merge_sort_core(array, 0, array.size());
    });
}