#include "all.h"
#include <vector>
#include <array>

void lsd_radix_sort(main_array& array)
{
	const uint radix_size = 256;

	static std::vector<item> buffer;
	static std::vector<uint> counts;
	buffer.resize(array.size());
	counts.resize(radix_size);
	//std::vector zero-initializes the elements

	for (uint radix_index = 0; radix_index < item::max_radix(radix_size); ++radix_index)
	{
		std::fill(counts.begin(), counts.end(), 0);

		for (auto& e : array)
		{
			uint radix = extract_radix(e, radix_index, radix_size);
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
			uint radix = extract_radix(e, radix_index, radix_size);
			uint& offset = counts[radix];
			buffer[offset] = e;
			++offset;
		}

		std::copy(buffer.begin(), buffer.end(), array.begin());
	}
}