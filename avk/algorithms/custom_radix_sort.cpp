#include "all.h"
#include "../internal/enforce.h"
#include <algorithm>
#include <iterator>
#include <cassert>
#include <bitset>

namespace stackless_american_flag_sort
{
	template <typename I, typename F>
	struct helper_type
	{
		using difference_type = typename std::iterator_traits<I>::difference_type;
		using size_type = std::make_unsigned_t<difference_type>;

		enum : uint_fast16_t
		{
			radix_size = 256,
			radix_size_mask = (uint_fast8_t)(radix_size - 1),
			radix_size_log2 = 8
		};

		inline static thread_local size_type counts[radix_size];
		inline static thread_local size_type offsets[radix_size];
		inline static thread_local size_type next_free[radix_size];

		static constexpr size_type shift(size_type value, size_type count)
		{
			for (; count != 0; --count)
				value >>= radix_size_log2;
			return value;
		}

		static constexpr void entry_point(I begin, I end, F& extract_digit, size_t digit_count)
		{
			size_t digit_index = digit_count - 1;
			I local_end = end;
			size_type m = 0;

			std::fill(std::begin(counts), std::end(counts), 0);
			std::fill(std::begin(offsets), std::end(offsets), 0);

			while (begin != end)
			{
				I next_end = begin;
				if (std::distance(begin, local_end) > 1)
				{
					for (uint_fast16_t i = 1; i != radix_size; ++i)
					{
						const size_t previous = counts[i - 1];
						counts[i] += previous;
						offsets[i] = previous;
					}

					std::copy(std::begin(offsets), std::end(offsets), std::begin(next_free));

					I i = begin;
					for (uint_fast8_t p = 0; p != radix_size_mask;)
					{
						uint_fast8_t next_p = p + 1;
						if (i >= begin + offsets[next_p])
						{
							p = next_p;
							continue;
						}
						const auto digit = extract_digit(*i, digit_index);
						if (digit == p)
						{
							++i;
							continue;
						}
						const I target = begin + next_free[digit];
						std::iter_swap(i, target);
						++next_free[digit];
					}

					next_end = begin + offsets[1];
					std::fill(std::begin(counts), std::end(counts), 0);
					std::fill(std::begin(offsets), std::end(offsets), 0);
				}

				if (digit_index == 0)
				{
					m += radix_size;
					for (size_type t = m >> radix_size_log2; (t & radix_size_mask) == 0; t >>= radix_size_log2)
						++digit_index;
					begin = local_end;
					for (; local_end != end && shift(local_end->value - m, digit_index + 1) == 0; ++local_end)
						++counts[extract_digit(*local_end, digit_index)];
				}
				else
				{
					local_end = next_end;
					--digit_index;
					for (I i = begin; i != local_end; ++i)
						++counts[extract_digit(*i, digit_index)];
				}
			}
		}
	};

	template <typename I, typename F>
	void sort(I begin, I end, F&& function, size_t digit_count)
	{
		helper_type<I, F>::entry_point(begin, end, function, digit_count);
	}
}

void custom_radix_sort(main_array array)
{
	stackless_american_flag_sort::sort(array.begin(), array.end(), extract_byte, item::max_radix());
}