#include "all.h"
#include <vector>
#include <array>
#include "sort_config.h"

void msd_radix_sort_helper(item* begin, item* end, size_t radix_index, size_t radix_size)
{
	const size_t size = (size_t)(end - begin);

	std::vector<size_t> offsets = {};
	std::vector<size_t> counts = {};
	counts.resize(radix_size);
	offsets.resize(radix_size);
	//std::vector zero-initializes the elements

	for (auto e = begin; e < end; ++e)
	{
		++counts[extract_radix(*e, radix_index, radix_size)];
	}

	size_t offset = 0;
	for (size_t i = 0; i < counts.size(); ++i)
	{
		offsets[i] = offset;
		offset += counts[i];
	}

	{
		std::vector<item> buffer;
		buffer.resize(size);

		for (auto e = begin; e < end; ++e)
		{
			const size_t radix = extract_radix(*e, radix_index, radix_size);
			size_t& o = offsets[radix];
			buffer[o] = *e;
			++o;
		}

		std::copy(buffer.begin(), buffer.end(), begin);
	}

	if (radix_index == 0)
		return;

	--radix_index;

	for (size_t e : counts)
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
	const size_t default_radix_size = sort_config::radix_size;
	msd_radix_sort_helper(array.begin(), array.end(), item::max_radix(default_radix_size) - 1, default_radix_size);
}