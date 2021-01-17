#include "all.h"
#include <vector>
#include <thread>

using std::vector;
using std::thread;

constexpr auto BITONIC_SORT_SEQUENTIAL_THRESHOLD = 128;

void bitonic_sort_merge(item* begin, item* end, bool ascending)
{
	if (end - begin < 2)
		return;
    uint size = end - begin;
    uint k = size / 2;
    if (size < BITONIC_SORT_SEQUENTIAL_THRESHOLD)
    {
        for (item* i = begin; i != begin + k; ++i)
            if (ascending == (*i > *(i + k)))
                swap(*i, *(i + k));
        bitonic_sort_merge(begin, begin + k, ascending);
        bitonic_sort_merge(begin + k, end, ascending);
    }
    else
    {
        for (item* i = begin; i != begin + k; ++i)
            if (ascending == (*i > *(i + k)))
                swap(*i, *(i + k));
        thread threads[] =
        {
            thread(bitonic_sort_merge, begin, begin + k, ascending),
            thread(bitonic_sort_merge, begin + k, end, ascending)
        };
        for (thread& t : threads)
            t.join();
    }
}

void bitonic_sort_kernel(item* begin, item* end, bool ascending)
{
    if (end - begin < 2)
        return;

    uint k = (end - begin) / 2;

    if (end - begin < BITONIC_SORT_SEQUENTIAL_THRESHOLD)
    {
        bitonic_sort_kernel(begin, begin + k, true);
        bitonic_sort_kernel(begin + k, end, false);
        bitonic_sort_merge(begin, end, ascending);
    }
    else
    {
        thread threads[] =
        {
            thread(bitonic_sort_kernel, begin, begin + k, true),
            thread(bitonic_sort_kernel, begin + k, end, false)
        };
        for (thread& t : threads)
            t.join();
        bitonic_sort_merge(begin, end, ascending);
    }
}

void bitonic_sort_parallel(main_array array)
{
    bitonic_sort_kernel(array.begin(), array.end(), true);
}