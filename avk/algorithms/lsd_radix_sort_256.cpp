#include "all.h"
#include <vector>
#include <array>

void lsd_radix_sort_256(main_array& array)
{
	std::vector<item> buffer;
	std::vector<uint> counts;
	buffer.resize(array.size());
	counts.resize(256);
	//std::vector zero-initializes the elements

	for (uint radix_index = 0; radix_index < item::max_radix(); ++radix_index)
	{
		std::fill(counts.begin(), counts.end(), 0);

		for (auto& e : array)
		{
			uint radix = extract_radix(e, radix_index);
			++counts[radix];
		}

		uint offset = 0;
		for (auto& e : counts)
		{
			uint tmp = e;
			e = offset;
			offset += tmp;
		}

		for (auto& e : array)
		{
			uint radix = extract_radix(e, radix_index);
			uint& offset = counts[radix];
			buffer[offset] = e;
			++offset;
		}

		std::copy(buffer.begin(), buffer.end(), array.begin());
	}
}