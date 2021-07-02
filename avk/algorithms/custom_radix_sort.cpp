#include "all.h"
#include <algorithm>
#include <iterator>
#include <cassert>
#include <bitset>

namespace detail
{
	template <typename I, typename F>
	struct msd_radix_sorter
	{
		using difference_type = typename std::iterator_traits<I>::difference_type;
		using size_type = std::make_unsigned_t<difference_type>;

		enum : uint_fast16_t
		{
			radix_size = 256,
			radix_size_mask = (uint_fast8_t)(radix_size - 1),
			radix_size_log2 = 8
		};

		F extract;

		constexpr msd_radix_sorter(F&& fn)
			: extract(std::forward<F>(fn))
		{
		}

		~msd_radix_sorter() = default;

		constexpr void sort(I begin, I end, size_t digit_index)
		{
			std::bitset<radix_size> presence = {};
			size_t counts[radix_size] = {};
			size_t offsets[radix_size] = {};
			while (true)
			{
				for (auto i = begin; i != end; ++i)
				{
					size_t key = extract(*i, digit_index);
					++counts[key];
					presence.set(key);
				}
				if (presence.count() > 1)
					break;
				if (digit_index == 0)
					return;
				++digit_index;
			}

			size_t k = 0;
			for (size_t i = 0; i != radix_size; ++i)
			{
				offsets[i] = k;
				k += counts[i];
			}

			auto i = begin;
			for (size_t partition_index = 0; partition_index < radix_size_mask;)
			{
				auto next_partition = partition_index + 1;
				if (!presence.test(partition_index) || i >= )
			}
		}
	};

	template <typename I, typename F>
	constexpr auto make_sorter(F&& fn)
	{
		return msd_radix_sorter<I, F>(std::forward<F>(fn));
	}
}

template <typename I, typename F>
static constexpr void msd_radix(I begin, I end, F&& extract)
{
	auto state = detail::make_sorter<I, F>(std::forward<F>(extract));
	state.sort(begin, end);
}

void custom_radix_sort(main_array array)
{
	msd_radix(array.begin(), array.end(), extract_byte);
}