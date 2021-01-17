#include "all.h"

void selection_sort(main_array array)
{
	for (uint i = 0; i < array.size() - 1; ++i)
	{
		uint min = i;
		for (uint j = i + 1; j < array.size(); ++j)
			if (array[j] < array[min])
				min = j;
		if (min != i)
			swap(array[i], array[min]);
	}
}