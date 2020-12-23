#include "all.h"
#include "../internal/enforce.h"
#include <algorithm>
#include <iterator>
#include <cassert>
#include <bitset>

namespace stripe_sort
{
	template <typename I, typename F>
	struct detail
	{
		static constexpr auto radix_size = 256;

		struct partition_info
		{
			I begin;
			I end;

			constexpr auto size() const
			{
				return std::distance(begin, end);
			}

			constexpr bool operator < (partition_info other) const
			{
				return size() < other.size();
			}
		};

		static void core(I begin, I end, size_t digit_index, F& extract_digit)
		{
			uint_fast16_t active_partition_count;
			partition_info partitions[256];
			static thread_local std::bitset<256> presence;
			static thread_local size_t counts[256];
			static thread_local I next_begin[256];

			while (true)
			{
				memset(counts, 0, sizeof(counts));
				presence.reset();
				for (I i = begin; i < end; ++i)
				{
					const auto digit = extract_digit(*i, digit_index);
					++counts[digit];
					presence.set(digit, true);
				}

				active_partition_count = presence.count();
				if (active_partition_count > 1)
				{
					I i = begin;
					for (uint_fast16_t p = 0; p < 256; ++p)
					{
						partitions[p].begin = i;
						i += counts[p];
						partitions[p].end = i;
					}
					break;
				}

				if (digit_index == 0)
					return;
				--digit_index;
			}

			for (uint_fast16_t p = 0; p < 256; ++p)
				next_begin[p] = partitions[p].begin;

			I i = begin;
			for (uint_fast16_t p = 0; p < 255;)
			{
				uint_fast16_t q = p + 1;
				if (!presence.test(p) || i >= partitions[q].begin)
				{
					p = q;
					continue;
				}
				const auto digit = extract_digit(*i, digit_index);
				assert(presence.test(digit));
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

			std::sort(std::begin(partitions), std::end(partitions));
			const auto partitions_end = partitions + 256;
			for (auto p = partitions_end - active_partition_count; p < partitions_end; ++p)
				core(p->begin, p->end, digit_index, extract_digit);
		}
	};

	template <typename I, typename F>
	void sort(I begin, I end, size_t last_digit, F&& function)
	{
		detail<I, F>::core(begin, end, last_digit, function);
	}
}

void custom_radix_sort(main_array& array)
{
	stripe_sort::sort(array.begin(), array.end(), item::max_radix() - 1, extract_byte);
}