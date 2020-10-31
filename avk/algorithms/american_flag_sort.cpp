#include "all.h"
#include <vector>

void american_flag_sort_256_helper(item* begin, item* end, uint radix_index, uint radix_size)
{
	const uint size = end - begin;

	std::vector<uint> offsets;
	std::vector<uint> counts;
	counts.resize(radix_size);
	offsets.resize(radix_size);
	//std::vector zero-initializes the elements

	for (auto e = begin; e < end; ++e)
	{
		++counts[extract_radix(*e, radix_index)];
	}

	uint offset = 0;
	for (uint i = 0; i < counts.size(); ++i)
	{
		offsets[i] = offset;
		offset += counts[i];
	}

	std::vector<uint> next_free = offsets;

	uint i = 0;
	for (uint partition = 0; partition < radix_size - 1;)
	{
		const uint next_partition = partition + 1;
		if (i >= offsets[next_partition])
		{
			partition = next_partition;
			continue;
		}
		const uint radix = extract_radix(begin[i], radix_index);
		if (radix == partition)
		{
			++i;
			continue;
		}
		uint& e = next_free[radix];
		swap(begin[e], begin[i]);
		++e;
	}

	if (radix_index == 0)
		return;

	--radix_index;
	for (uint e : counts)
	{
		if (e > 0)
		{
			american_flag_sort_256_helper(begin, begin + e, radix_index, radix_size);
			begin += e;
		}
	}
}

void american_flag_sort(main_array& array)
{
	const uint default_radix_size = 256;
	american_flag_sort_256_helper(array.begin(), array.end(), item::max_radix(default_radix_size) - 1, default_radix_size);
}