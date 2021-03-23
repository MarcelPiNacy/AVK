#include "all.h"
#include <algorithm>

void parallel_merge_sort(main_array array)
{
	as_parallel([=]()
	{
		uint base_run_size = 16;
		uint size = array.size();

		parallel_for<uint>(0, size / base_run_size, [=](uint base_index)
		{
			sint begin = (base_index * base_run_size);
			sint end = begin + base_run_size;
			for (sint i = begin + 1; i < end; ++i)
			{
				item tmp = array[i];
				sint j = i - 1;
				for (; j >= begin && array[j] > tmp; --j)
					array[j + 1] = array[j];
				array[j + 1] = tmp;
			}
		});

		auto begin = array.begin();
		for (uint run_size = base_run_size; run_size < size;)
		{
			uint next_run_size = run_size * 2;
			uint run_count = size / run_size;

			parallel_for<uint>(0, run_count, [=](uint base_index)
			{
				uint begin_offset = base_index * next_run_size;
				uint middle_offset = begin_offset + run_size;
				if (middle_offset >= size)
					return;
				uint end_offset = middle_offset + run_size;
				if (end_offset > size)
					end_offset = size;
				std::inplace_merge(begin + begin_offset, begin + middle_offset, begin + end_offset);
			});

			run_size = next_run_size;
		}
	});
}