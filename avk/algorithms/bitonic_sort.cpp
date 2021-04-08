#include "all.h"

void bitonic_sort(main_array array)
{
	for (size_t k = 2; k <= array.size(); k *= 2)
	{
		for (size_t j = k / 2; j > 0; j /= 2)
		{
			for (size_t i = 0; i < array.size(); ++i)
			{
				size_t l = i ^ j;
				if (l > i)
				{
					if (((i & k) == 0 && (array[i] > array[l])) ||
						((i & k) != 0 && array[i] < array[l]))
					{
						swap(array[i], array[l]);
					}
				}
			}
		}
	}
}