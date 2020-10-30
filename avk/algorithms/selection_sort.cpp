#include "all.h"

void selection_sort(main_array& array)
{
	for (int i = 0; i < (int)array.size() - 1; ++i)
	{
		int min = i;
		for (int j = i + 1; j < (int)array.size(); ++j)
			if (array[j] < array[min])
				min = j;
		if (min != i)
			swap(array[i], array[min]);
	}
}