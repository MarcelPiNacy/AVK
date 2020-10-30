#include "all.h"

void bitonic_sort(main_array& array)
{
	for (int k = 2; k <= array.size(); k *= 2)
	{
		for (int j = k / 2; j > 0; j /= 2)
		{
			for (int i = 0; i < array.size(); ++i)
			{
				int l = i ^ j;
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