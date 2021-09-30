#include "all.h"
#include <Comet.hpp>

static void pairwise_sorting_network_parallel_step(main_array array, size_t begin, size_t end, size_t gap)
{
    if (begin == end - gap)
        return;

    size_t n = 0;
    for (size_t i = begin + gap; i < end; i += (2 * gap))
        ++n;

    Comet::ForEach<size_t>(0, n, [&](size_t i)
    {
        i *= gap * 2;
        i += begin + gap;
        compare_swap(array, i - gap, i);
    });

    if ((((end - begin) / gap) & 1) == 0)
    {
        size_t params[2][2] =
        {
            { begin, end },
            { begin + gap, end + gap }
        };

        Comet::ForEach(0, 2, [&](int i)
        {
            pairwise_sorting_network_parallel_step(array, params[i][0], params[i][1], gap * 2);
        });
    }
    else
    {
        size_t params[2][2] =
        {
            { begin, end + gap },
            { begin + gap, end }
        };

        Comet::ForEach(0, 2, [&](int i)
        {
            pairwise_sorting_network_parallel_step(array, params[i][0], params[i][1], gap * 2);
        });
    }

    size_t k = 1;
    while (k < ((end - begin) / gap))
        k = (k * 2) + 1;

    n = 0;
    for (size_t i = begin + gap; i + gap < end; i += (2 * gap))
        ++n;

    for (size_t j = k; j > 1;)
    {
        j /= 2;
        Comet::ForEach<size_t>(0, n, [&](size_t i)
        {
            i *= 2 * gap;
            i += begin + gap;
            if (i + (j * gap) < end)
                compare_swap(array, i, i + (j * gap));
        });
    }
}



void pairwise_sorting_network_parallel(main_array array)
{
    array.begin_parallel_sort();
    pairwise_sorting_network_parallel_step(array, 0, array.size(), 1);
    array.end_parallel_sort();
}