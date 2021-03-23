#include "all.h"

void odd_even_merge_sort(main_array array)
{
	for (uint run_size = 1; run_size < array.size(); run_size *= 2)
	{
		for (uint block_size = run_size; block_size > 0; block_size /= 2)
		{
			for (uint j = block_size & (run_size - 1); j < array.size() - block_size; j += (2 * block_size))
			{
				for (uint i = 0; i < block_size; ++i)
				{
					if ((i + j) / (run_size * 2) == (i + j + block_size) / (run_size * 2))
					{
						if (array[i + j] > array[i + j + block_size])
						{
							swap(array[i + j], array[i + j + block_size]);
						}
					}
				}
			}
		}
	}
}