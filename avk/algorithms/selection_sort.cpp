#include "all.h"

void selection_sort(main_array array)
{
	for (size_t i = 0; i < array.size() - 1; ++i)
	{
		size_t min = i;
		for (size_t j = i + 1; j < array.size(); ++j)
			if (array[j] < array[min])
				min = j;
		if (min != i)
			swap(array[i], array[min]);
	}
}