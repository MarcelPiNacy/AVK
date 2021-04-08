#include "all.h"
#include <algorithm>

void parallel_merge_sort(main_array array)
{
	as_parallel([=]()
	{
		size_t base_run_size = 16;
		size_t size = array.size();

		parallel_for<size_t>(0, size / base_run_size, [=](size_t base_index)
		{
			ptrdiff_t begin = (base_index * base_run_size);
			ptrdiff_t end = begin + base_run_size;
			for (ptrdiff_t i = begin + 1; i < end; ++i)
			{
				item tmp = array[i];
				ptrdiff_t j = i - 1;
				for (; j >= begin && array[j] > tmp; --j)
					array[j + 1] = array[j];
				array[j + 1] = tmp;
			}
		});

		auto begin = array.begin();
		for (size_t run_size = base_run_size; run_size < size;)
		{
			size_t next_run_size = run_size * 2;
			size_t run_count = size / run_size;

			parallel_for<size_t>(0, run_count, [=](size_t base_index)
			{
				size_t begin_offset = base_index * next_run_size;
				size_t middle_offset = begin_offset + run_size;
				if (middle_offset >= size)
					return;
				size_t end_offset = middle_offset + run_size;
				if (end_offset > size)
					end_offset = size;
				std::inplace_merge(begin + begin_offset, begin + middle_offset, begin + end_offset);
			});

			run_size = next_run_size;
		}
	});
}