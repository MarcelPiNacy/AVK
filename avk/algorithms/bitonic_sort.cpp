#include "all.h"

void bitonic_sort(main_array& array)
{
	for (uint k = 2; k <= array.size(); k *= 2)
	{
		for (uint j = k / 2; j > 0; j /= 2)
		{
			for (uint i = 0; i < array.size(); ++i)
			{
				uint l = i ^ j;
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