#include "all.h"
#include <vector>
#include <array>

void msd_radix_sort_helper(item* begin, item* end, uint radix_index, uint radix_size)
{
	const uint size = (uint)(end - begin);

	std::vector<uint> offsets = {};
	std::vector<uint> counts = {};
	counts.resize(radix_size);
	offsets.resize(radix_size);
	//std::vector zero-initializes the elements

	for (auto e = begin; e < end; ++e)
	{
		++counts[extract_radix(*e, radix_index, radix_size)];
	}

	uint offset = 0;
	for (uint i = 0; i < counts.size(); ++i)
	{
		offsets[i] = offset;
		offset += counts[i];
	}

	{
		std::vector<item> buffer;
		buffer.resize(size);

		for (auto e = begin; e < end; ++e)
		{
			const uint radix = extract_radix(*e, radix_index, radix_size);
			uint& o = offsets[radix];
			buffer[o] = *e;
			++o;
		}

		std::copy(buffer.begin(), buffer.end(), begin);
	}

	if (radix_index == 0)
		return;

	--radix_index;

	for (uint e : counts)
	{
		if (e > 0)
		{
			msd_radix_sort_helper(begin, begin + e, radix_index, radix_size);
			begin += e;
		}
	}
}

void msd_radix_sort(main_array array)
{
	const uint default_radix_size = sort_config::radix_size;
	msd_radix_sort_helper(array.begin(), array.end(), item::max_radix(default_radix_size) - 1, default_radix_size);
}