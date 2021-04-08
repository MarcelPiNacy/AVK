#include "all.h"



static void pairwise_sorting_network_step(main_array array, size_t begin, size_t end, size_t gap)
{
    if (begin == end - gap)
        return;

    for (size_t i = begin + gap; i < end; i += (2 * gap))
        compare_swap(array, i - gap, i);

    if ((((end - begin) / gap) & 1) == 0)
    {
        pairwise_sorting_network_step(array, begin, end, gap * 2);
        pairwise_sorting_network_step(array, begin + gap, end + gap, gap * 2);
    }
    else
    {
        pairwise_sorting_network_step(array, begin, end + gap, gap * 2);
        pairwise_sorting_network_step(array, begin + gap, end, gap * 2);
    }

    size_t k = 1;
    while (k < ((end - begin) / gap))
        k = (k * 2) + 1;

    for (size_t j = k; j > 1;)
    {
        j /= 2;
    for (size_t i = begin + gap; i + gap < end; i += (2 * gap))
    {
            if (i + (j * gap) < end)
                compare_swap(array, i, i + (j * gap));
        }
    }
}

void pairwise_sorting_network_recursive(main_array array)
{
    pairwise_sorting_network_step(array, 0, array.size(), 1);
}