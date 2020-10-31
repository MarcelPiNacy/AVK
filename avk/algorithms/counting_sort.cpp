#include "all.h"
#include <vector>

void counting_sort(main_array& array)
{
	std::vector<uint> counts;
	std::vector<item> buffer;
	buffer.resize(array.size());

	uint max = 0;
	for (uint i = 0; i < array.size(); ++i)
	{
		if (array[i].value > max)
			max = array[i].value;
		buffer[i] = array[i];
		stats::add_read();
		stats::add_comparisson();
	}

	counts.resize(max);
	for (item& e : buffer)
		++counts[e.value];
	uint offset = 0;
	for (uint& e : counts)
	{
		uint k = e;
		e += offset;
		offset += k;
	}
	for (item& e : buffer)
	{
		uint& k = counts[e.value];
		array[k] = e;
		++k;
	}
}