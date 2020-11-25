#include "all.h"
#include "sort_utility.h"

void simple_flash_sort(item* begin, item* end)
{
	const uint size = end - begin;

	auto min = begin;
	auto max = begin;
	for (auto i = begin; i < end; ++i)
	{
		if (*i < *min)
			min = i;
		if (*i > *max)
			max = i;
	}

	swap(*begin, *min);
	swap(*(end - 1), *max);
	const uint range = end[-1].value - begin->value;

	auto i = begin;
	while (i < end)
	{
		const uint target = (i->value - min->value) / size;
		swap(begin[target], *i);
		++i;
	}
}

void simple_flash_sort(main_array& array)
{
	simple_flash_sort(array.begin(), array.end());
}