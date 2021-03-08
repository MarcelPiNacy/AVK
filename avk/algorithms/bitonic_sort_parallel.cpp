#include "all.h"

constexpr auto BITONIC_SORT_SEQUENTIAL_THRESHOLD = 128;

void bitonic_sort_merge(main_array array, uint begin, uint end, bool ascending)
{
	if (end - begin < 2)
		return;
    uint size = (uint)(end - begin);
    AVK_ASSERT(size <= array.size());
    uint k = size / 2;
    AVK_ASSERT(k <= array.size() / 2);
    if (size < BITONIC_SORT_SEQUENTIAL_THRESHOLD)
    {
        for (uint i = begin; i < begin + k; ++i)
            if (ascending == (array[i] > array[i + k]))
                swap(array, i, i + k);

        bitonic_sort_merge(array, begin, begin + k, ascending);
        bitonic_sort_merge(array, begin + k, end, ascending);
    }
    else
    {
        parallel_for(begin, begin + k, [&](uint i)
        {
            if (ascending == (array[i] > array[i + k]))
                swap(array, i, i + k);
        });
        
        parallel_for(0, 2, [&](int i)
        {
            uint b = begin;
            uint e = begin + k;
            if (i != 0)
                b += k;
            if (i != 0)
                e += k;
            bitonic_sort_merge(array, b, e, ascending);
        });
    }
}

void bitonic_sort_kernel(main_array array, uint begin, uint end, bool ascending)
{
    uint size = (uint)(end - begin);
    AVK_ASSERT(size <= array.size());

    if (end - begin < 2)
        return;

    uint k = size / 2;
    AVK_ASSERT(k <= array.size() / 2);

    if (end - begin < BITONIC_SORT_SEQUENTIAL_THRESHOLD)
    {
        bitonic_sort_kernel(array, begin, begin + k, true);
        bitonic_sort_kernel(array, begin + k, end, false);
        bitonic_sort_merge(array, begin, end, ascending);
    }
    else
    {
        parallel_for(0, 2, [&](int i)
        {
            uint b = begin;
            uint e = begin + k;
            if (i != 0)
                b += k;
            if (i != 0)
                e += k;
            bitonic_sort_kernel(array, b, e, i == 0);
        });

        bitonic_sort_merge(array, begin, end, ascending);
    }
}

void bitonic_sort_parallel(main_array array)
{
    run_as_parallel([&]() { bitonic_sort_kernel(array, 0, array.size(), true); });
}