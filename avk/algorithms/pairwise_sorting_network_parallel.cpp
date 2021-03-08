#include "all.h"

static void pairwise_sorting_network_parallel_kernel(main_array array, uint start, uint end, uint gap)
{
    if (start == end - gap)
        return;

    int b = start + gap;
    while (b < end)
    {
        compare_swap(array, b - gap, b);
        b += (2 * gap);
    }

    if (((end - start) / gap) % 2 == 0)
    {
        parallel_for(0, 2, [=](int i)
        {
            if (i == 0)
                pairwise_sorting_network_parallel_kernel(array, start + gap, end + gap, gap * 2);
            else
                pairwise_sorting_network_parallel_kernel(array, start, end, gap * 2);
        });
    }
    else
    {
        parallel_for(0, 2, [=](int i)
        {
            if (i == 0)
                pairwise_sorting_network_parallel_kernel(array, start, end + gap, gap * 2);
            else
                pairwise_sorting_network_parallel_kernel(array, start + gap, end, gap * 2);
        });
    }

    int a = 1;
    while (a < ((end - start) / gap))
    {
        a = (a * 2) + 1;
    }
    b = start + gap;
    while (b + gap < end)
    {
        int c = a;
        while (c > 1)
        {
            c /= 2;
            if (b + (c * gap) < end)
                compare_swap(array, b, b + (c * gap));
        }
        b += (2 * gap);
    }
}

void pairwise_sorting_network_parallel(main_array array)
{
    run_as_parallel([=]() { pairwise_sorting_network_parallel_kernel(array, 0, array.size(), 1); });
}