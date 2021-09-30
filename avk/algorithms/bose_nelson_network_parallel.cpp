#include "all.h"
#include <Comet.hpp>

static void bose_nelson_merge(main_array array, size_t start1, size_t len1, size_t start2, size_t len2)
{
    if (len1 == 1 && len2 == 1)
    {
        compare_swap(array, start1, start2);
    }
    else if (len1 == 1 && len2 == 2)
    {
        compare_swap(array, start1, start2 + 1);
        compare_swap(array, start1, start2);
    }
    else if (len1 == 2 && len2 == 1)
    {
        compare_swap(array, start1, start2);
        compare_swap(array, start1 + 1, start2);
    }
    else
    {
        size_t mid1 = len1 / 2;
        size_t mid2 = len1 % 2 == 1 ? len2 / 2 : (len2 + 1) / 2; // stfu msvc
        size_t params[3][4] =
        {
            { start1, mid1, start2, mid2 },
            { start1 + mid1, len1 - mid1, start2 + mid2, len2 - mid2 },
            { start1 + mid1, len1 - mid1, start2, mid2 }
        };

        for (size_t i = 0; i != 3; ++i)
        {
            bose_nelson_merge(array, params[i][0], params[i][1], params[i][2], params[i][3]);
        }
    }
}

static void bose_nelson_core(main_array array, size_t start, size_t length)
{
    if (length < 2)
        return;

    size_t mid = length / 2;

    size_t params[2][2] =
    {
        { start, mid },
        { start + mid, length - mid }
    };
    
    Comet::ForEach<size_t>(0, 2, [&](size_t i)
    {
        bose_nelson_core(array, params[i][0], params[i][1]);
    });

    bose_nelson_merge(array, start, mid, start + mid, length - mid);
}

void bose_nelson_network_parallel(main_array array)
{
    array.begin_parallel_sort();
    bose_nelson_core(array, 0, array.size());
    array.end_parallel_sort();
}