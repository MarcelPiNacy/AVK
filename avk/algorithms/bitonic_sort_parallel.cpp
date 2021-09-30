#include "all.h"
#include <Comet.hpp>

void bitonic_sort_merge(main_array array, size_t begin, size_t end, bool ascending)
{
	if (end - begin < 2)
		return;
    size_t size = (size_t)(end - begin);
    AVK_ASSERT(size <= array.size());
    size_t k = size / 2;
    AVK_ASSERT(k <= array.size() / 2);

    Comet::ForEach(begin, begin + k, [&](size_t i)
    {
        if (ascending == (array[i] > array[i + k]))
            swap(array, i, i + k);
    });

    Comet::ForEach(0, 2, [&](int i)
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

    Comet::ForEach(0, 2, [&](int i)
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
    array.begin_parallel_sort();
    bitonic_sort_kernel(array, 0, array.size(), true);
    array.end_parallel_sort();
}