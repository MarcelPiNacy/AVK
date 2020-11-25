#include "all.h"
#include <algorithm>



constexpr item* binary_search(item* begin, item* end, item* target)
{
	while (true)
	{
		const auto delta = std::distance(begin, end);
		if (delta <= 0)
			break;
		item* p = begin + delta / 2;
		if (*p == *target)
			return p;
		if (*p > *target)
			end = p;
		else
			begin = p + 1;
	}
	return end;
}

void gambit_insertion_sort(main_array& array)
{
	uint offset = 1;
	while (offset * offset < array.size())
		offset *= 2;

	auto begin = array.begin();
	auto end = array.end();
	auto i = begin + offset;

	while (i < end)
	{
		auto target = binary_search(begin, i, i);
		auto tmp = *i;
		auto j = i - 1;
		while (j >= target && *j > tmp)
		{
			j[1] = *j;
			--j;
		}
		j[1] = tmp;
		++i;
	}

	insertion_sort(array);
}