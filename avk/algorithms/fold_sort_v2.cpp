#include "all.h"
#include "../internal/prng.h"



void fold(item* low, item* high)
{
	while (low < high)
	{
		if (*low > * high)
			swap(*low, *high);
		++low;
		--high;
	}
}

void fold_sort_v2(main_array& array)
{
	for (auto i = array.size(); i > 0; i /= 2)
	{
		for (auto low = array.begin(); low < array.end();)
		{
			auto high = low + i;
			fold(low, high - 1);
			low = high;
		}
	}
}