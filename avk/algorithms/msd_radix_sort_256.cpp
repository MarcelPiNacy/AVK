#include "all.h"
#include <vector>
#include <array>

void msd_radix_sort_256_helper(item* begin, item* end, uint radix_index)
{
	const uint size = end - begin;

	std::vector<uint> offsets = {};
	std::vector<uint> counts = {};
	counts.resize(256);
	offsets.resize(256);
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

	{
		std::vector<item> buffer;
		buffer.resize(size);

		for (auto e = begin; e < end; ++e)
		{
			const uint radix = extract_radix(*e, radix_index);
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
			msd_radix_sort_256_helper(begin, begin + e, radix_index);
			begin += e;
		}
	}
}

void msd_radix_sort_256(main_array& array)
{
	msd_radix_sort_256_helper(array.begin(), array.end(), item::max_radix() - 1);
}