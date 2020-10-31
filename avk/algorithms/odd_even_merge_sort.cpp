#include "all.h"

void odd_even_merge_sort(main_array& array)
{
	for (uint p = 1; p < array.size(); p *= 2)
	{
		for (uint k = p; k > 0; k /= 2)
		{
			for (uint j = k & (p - 1); j < array.size() - k; j += (2 * k))
			{
				for (uint i = 0; i < k; ++i)
				{
					if ((i + j) / (p * 2) == (i + j + k) / (p * 2))
					{
						if (array[i + j] > array[i + j + k])
						{
							swap(array[i + j], array[i + j + k]);
						}
					}
				}
			}
		}
	}
}