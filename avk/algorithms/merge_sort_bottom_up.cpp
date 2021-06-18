#include "all.h"
#include <algorithm>

#ifdef AVK_USE_PARITY_MERGE
#include <vector>

template <typename I>
static void parity_merge(I begin, I middle, I end)
{
    std::vector<typename std::iterator_traits<I>::value_type> buffer;
    buffer.resize(std::distance(begin, end));
    auto dest = buffer.begin();
    I from = begin;
    size_t left = std::distance(begin, middle);
    AVK_ASSERT(std::distance(middle, end) == left);
#ifdef AVK_SPLIT_PARITY_MERGE
    size_t prior = left;
#endif

    size_t head, tail, left_head, left_tail, right_head, right_tail;

    left_head = 0;
    right_head = left;

    left_tail = left - 1;
    right_tail = left_tail + left;

    head = 0;
    tail = right_tail;

    do
    {
        if (from[left_head] <= from[right_head])
        {
            dest[head++] = from[left_head++];
        }
        else
        {
            dest[head++] = from[right_head++];
        }
#ifdef AVK_SPLIT_PARITY_MERGE
    } while (--left);
    left = prior;
    do
    {
#endif
        if (from[left_tail] > from[right_tail])
        {
            dest[tail--] = from[left_tail--];
        }
        else
        {
            dest[tail--] = from[right_tail--];
        }
    } while (--left);

    std::copy(buffer.begin(), buffer.end(), begin);
}
#endif



void merge_sort_bottom_up(main_array array)
{
	for (size_t i = 0; i < array.size(); i += 2)
		if (array[i] > array[i + 1])
			swap(array[i], array[i + 1]);

	size_t merge_size = 2;
	while (merge_size < array.size())
	{
		size_t next = merge_size * 2;
		for (size_t i = 0; i < array.size(); i += next)
		{
			auto p = array.begin() + i;
#ifdef AVK_USE_PARITY_MERGE
            parity_merge(p, p + merge_size, p + next);
#else
            std::inplace_merge(p, p + merge_size, p + next);
#endif
		}
		merge_size = next;
	}
}