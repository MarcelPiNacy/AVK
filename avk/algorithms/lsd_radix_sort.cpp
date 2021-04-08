#include "all.h"
#include <vector>
#include <array>
#include "sort_config.h"

void lsd_radix_sort(main_array array)
{
	const size_t radix_size = sort_config::radix_size;

	static std::vector<item> buffer;
	static std::vector<size_t> counts;
	buffer.resize(array.size());
	counts.resize(radix_size);
	//std::vector zero-initializes the elements

	for (size_t radix_index = 0; radix_index < item::max_radix(radix_size); ++radix_index)
	{
		std::fill(counts.begin(), counts.end(), 0);

		for (auto& e : array)
		{
			size_t radix = extract_radix(e, radix_index, radix_size);
			++counts[radix];
		}

		size_t offset = 0;
		for (auto& e : counts)
		{
			size_t tmp = e;
			e = offset;
			offset += tmp;
		}

		for (auto& e : array)
		{
			size_t radix = extract_radix(e, radix_index, radix_size);
			size_t& offset = counts[radix];
			buffer[offset] = e;
			++offset;
		}

		std::copy(buffer.begin(), buffer.end(), array.begin());
	}
}