#pragma once
#include "all.h"
#include "sort_utility.h"

static void halver(main_array& array, uint low, uint high)
{
	while (low < high)
	{
		compare_swap(array, low, high);
		++low;
		--high;
	}
}

void another_another_sorting_network(main_array& array)
{
}