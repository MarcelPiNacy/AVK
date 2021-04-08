#include "all.h"
#include <vector>

void counting_sort(main_array array)
{
	std::vector<size_t> counts;
	std::vector<item> buffer;
	buffer.resize(array.size());

	size_t max = 0;
	for (size_t i = 0; i < array.size(); ++i)
	{
		if (array[i].value > max)
			max = array[i].value;
		buffer[i] = array[i];
		sort_stats::add_read();
		sort_stats::add_comparisson();
	}

	counts.resize(max + 1);
	for (item& e : buffer)
		++counts[e.value];
	size_t offset = 0;
	for (size_t& e : counts)
	{
		size_t k = e;
		e = offset;
		offset += k;
	}
	for (item& e : buffer)
	{
		size_t& k = counts[e.value];
		array[k] = e;
		++k;
	}
}