#include "all.h"
#include <algorithm>
#include <vector>

void std_merge_sort(main_array& array)
{
	std::vector<item> buffer;
	buffer.resize(array.size());

	for (uint i = 0; i < array.size(); i += 2)
		if (array[i] > array[i + 1])
			swap(array[i], array[i + 1]);

	uint merge_size = 2;
	while (merge_size < array.size())
	{
		uint next = merge_size * 2;
		auto out = buffer.begin();
		for (uint i = 0; i < array.size(); i += next)
		{
			auto p = array.begin() + i;
			std::merge(p, p + merge_size, p + merge_size, p + next, out);
			out += next;
		}
		std::copy(buffer.begin(), buffer.end(), array.begin());
		merge_size = next;
	}
}