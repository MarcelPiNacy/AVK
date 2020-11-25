#include "all.h"
#include "sort_utility.h"

bool compare_swap(main_array& array, uint left, uint right)
{
	const bool r = array[right] < array[left];
	if (r)
		swap(array[right], array[left]);
	return r;
}

/*
*	TRIVIAL HALVER: 
*/
void trivial_halver(main_array& array, uint low, uint high)
{
	while (low < high)
	{
		compare_swap(array, low, high);
		++low;
		--high;
	}
}

void halver(main_array& array, uint low, uint high)
{
	trivial_halver(array, low, high);
}

void another_sorting_network(main_array& array)
{
	const auto size = array.size();
	for (auto i = size; i > 0; i /= 2)
	{
		for (auto j = size; j >= i; j /= 2)
		{
			for (auto low = 0; low < array.size();)
			{
				auto high = low + j;
				halver(array, low, high - 1);
				low = high;
			}
		}
	}
}