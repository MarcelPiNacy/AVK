#include "all.h"
#include <vector>



namespace ctree_sort_detail
{
	enum class compare_info
	{
		EQUAL,
		LESS_THAN,
		GREATER_THAN,
	};

	template <typename I, typename L>
	struct helper_type
	{
		L lookup;
		size_t lookup_size;
		size_t lookup_mask_count;
		size_t array_size;

		void set_lookup(size_t x, size_t y, compare_info value)
		{
			size_t index = x + y * array_size;
			size_t mask_index = index >> 6;
			uint8_t bit_index = (index & 63) / 2;
			*std::next(lookup, mask_index) |= (uint64_t)value << bit_index;
		}

		compare_info query_lookup(size_t x, size_t y) const
		{
			size_t index = x + y * array_size;
			size_t mask_index = index >> 6;
			uint8_t bit_index = (index & 63) / 2;
			uint64_t mask = *std::next(lookup, mask_index);
			mask >>= bit_index;
			mask &= 0b11;
			return (compare_info)mask;
		}

		void init_lookup()
		{
			std::fill(lookup, lookup + lookup_mask_count, 0);
		}

		void sort(I begin, I end, L lookup, size_t lookup_size)
		{
			this->lookup_size = lookup_size;
			this->lookup_mask_count = lookup_size / 32;
			this->array_size = (size_t)std::distance(begin, end);
			this->lookup = lookup;
		}
	};

	template <typename I>
	constexpr size_t lookup_size(I begin, I end)
	{
		auto size = std::distance(begin, end);
		return ((size_t)size * size - size) / 64;
	}

	template <typename I, typename L>
	void sort(I begin, I end, L lookup)
	{
		helper_type<I, L> state;
		state.sort(begin, end, lookup, lookup_size(begin, end));
	}
}



void ctree_sort(main_array array)
{
	size_t lookup_size = ctree_sort_detail::lookup_size(array.begin(), array.end());
	std::vector<uint64_t> lookup;
	lookup.resize(lookup_size);
	ctree_sort_detail::sort(array.begin(), array.end(), lookup.begin());
}