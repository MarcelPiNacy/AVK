#include "all.h"
#include <vector>

#include <algorithm>
#include <iterator>
#include <cassert>
#include <bitset>
#include <limits>
#include <bitset>
#include <cstdint>
#include <iterator>
#include <type_traits>

namespace detail::american_flag_sort
{
	template <size_t K>
	using uint_least_fast_t =
		std::conditional_t<(K > std::numeric_limits<std::uint32_t>::max()), uint_fast64_t,
		std::conditional_t<(K > std::numeric_limits<std::uint16_t>::max()), uint_fast32_t,
		std::conditional_t<(K > std::numeric_limits<std::uint8_t>::max()), uint_fast16_t, uint_fast8_t>>>;

	template <size_t Radix, typename I, typename F>
	struct helper_type
	{
		using radix_index_type = uint_least_fast_t<Radix>;
		using size_type = std::make_unsigned_t<typename std::iterator_traits<I>::difference_type>;

		struct partition_info
		{
			I begin;
			I end;

			constexpr size_type size() const
			{
				return (size_type)std::distance(begin, end);
			}

			constexpr bool operator < (partition_info other) const
			{
				return size() < other.size();
			}
		};

		static constexpr void core(I begin, I end, size_t digit_index, F& extract_digit)
		{
			radix_index_type active_partition_count = 0;
			partition_info partitions[Radix] = {};

			/*static thread_local*/ std::bitset<Radix> presence = {};
			/*static thread_local*/ size_t counts[Radix] = {};
			/*static thread_local*/ I next_begin[Radix] = {};

			while (true)
			{
				std::fill(std::begin(counts), std::end(counts), (size_t)0);
				presence.reset();

				for (I i = begin; i < end; ++i)
				{
					const radix_index_type digit = extract_digit(*i, digit_index);
					++counts[digit];
					presence.set(digit, true);
				}

				active_partition_count = presence.count();

				if (active_partition_count > 1)
					break;

				if (digit_index == 0)
					return;
				--digit_index;
			}

			I i = begin;
			for (radix_index_type p = 0; p < Radix; ++p)
			{
				partitions[p].begin = i;
				next_begin[p] = i;
				i += counts[p];
				partitions[p].end = i;
			}

			i = begin;
			for (radix_index_type p = 0; p < Radix - 1;)
			{
				radix_index_type q = p + 1;
				if (!presence.test(p) || i >= partitions[q].begin)
				{
					p = q;
					continue;
				}
				const radix_index_type digit = extract_digit(*i, digit_index);
				if (digit == p)
				{
					++i;
					continue;
				}
				std::iter_swap(i, next_begin[digit]);
				++next_begin[digit];
			}

			if (digit_index == 0)
				return;
			--digit_index;

			std::stable_sort(std::begin(partitions), std::end(partitions));
			const partition_info* const partitions_end = partitions + Radix;
			for (auto p = partitions_end - active_partition_count; p < partitions_end; ++p)
				core(p->begin, p->end, digit_index, extract_digit);
		}
	};

	template <size_t Radix, typename I, typename F>
	constexpr void sort(I begin, I end, size_t last_digit, F&& function)
	{
		helper_type<Radix, I, F>::core(begin, end, last_digit, function);
	}
}

template <size_t MaxDigits, size_t RadixSize = 256, typename RandomAccessIterator, typename ExtractDigit>
constexpr void american_flag_sort(RandomAccessIterator begin, RandomAccessIterator end, ExtractDigit&& extract_digit)
{
	static_assert(
		std::is_same<typename std::iterator_traits<RandomAccessIterator>::iterator_category, std::random_access_iterator_tag>::value,
		"American Flag Sort: Only random access iterators are suported.");
	detail::american_flag_sort::sort<RadixSize>(begin, end, MaxDigits - 1, extract_digit);
}

void american_flag_sort(main_array array)
{
	const uint default_radix_size = sort_config::radix_size;
	american_flag_sort<4>(array.begin(), array.end(), extract_byte);
}