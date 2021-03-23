#include "all.h"

static void bose_nelson_merge(main_array array, uint start1, uint len1, uint start2, uint len2)
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
        uint mid1 = len1 / 2;
        uint mid2 = len1 % 2 == 1 ? len2 / 2 : (len2 + 1) / 2;
        uint params[3][4] =
        {
            { start1 , mid1 , start2 , mid2 },
            { start1 + mid1 , len1 - mid1 , start2 + mid2 , len2 - mid2 },
            { start1 + mid1 , len1 - mid1 , start2 , mid2 }
        };

        parallel_for<uint>(0, 3, [&](uint i)
        {
            bose_nelson_merge(array, params[i][0], params[i][1], params[i][2], params[i][3]);
        });
    }
}

static void bose_nelson_core(main_array array, uint start, uint length)
{
    if (length < 2)
        return;

    uint mid = length / 2;

    uint params[2][2] =
    {
        { start, mid },
        { start + mid, length - mid }
    };
    
    parallel_for<uint>(0, 2, [&](uint i)
    {
        bose_nelson_core(array, params[i][0], params[i][1]);
    });

    bose_nelson_merge(array, start, mid, start + mid, length - mid);
}

void bose_nelson_network_parallel(main_array array)
{
    as_parallel([=]() { bose_nelson_core(array, 0, array.size()); });
}