#include "all.h"

void bitonic_sort_merge(main_array array, size_t begin, size_t end, bool ascending)
{
	if (end - begin < 2)
		return;
    size_t size = (size_t)(end - begin);
    AVK_ASSERT(size <= array.size());
    size_t k = size / 2;
    AVK_ASSERT(k <= array.size() / 2);

    parallel_for(begin, begin + k, [&](size_t i)
    {
        if (ascending == (array[i] > array[i + k]))
            swap(array, i, i + k);
    });

    parallel_for(0, 2, [&](int i)
    {
        size_t b = begin;
        size_t e = begin + k;
        if (i != 0)
            b += k;
        if (i != 0)
            e += k;
        bitonic_sort_merge(array, b, e, ascending);
    });
}

void bitonic_sort_kernel(main_array array, size_t begin, size_t end, bool ascending)
{
    size_t size = (size_t)(end - begin);
    AVK_ASSERT(size <= array.size());

    if (end - begin < 2)
        return;

    size_t k = size / 2;
    AVK_ASSERT(k <= array.size() / 2);

    parallel_for(0, 2, [&](int i)
    {
        size_t b = begin;
        size_t e = begin + k;
        if (i != 0)
            b += k;
        if (i != 0)
            e += k;
        bitonic_sort_kernel(array, b, e, i == 0);
    });

    bitonic_sort_merge(array, begin, end, ascending);
}

void bitonic_sort_parallel(main_array array)
{
    as_parallel([&]() { bitonic_sort_kernel(array, 0, array.size(), true); });
}