#include "all.h"
#include "../internal/enforce.h"
#include <algorithm>
#include <iterator>
#include <cassert>

namespace stripe_sort
{
	namespace detail
	{
		enum : size_t
		{
			RADIX_SIZE = 256,
			ACTIVE_MASK_COUNT = RADIX_SIZE / 64,
			ACTIVE_MASK_SIZE = ACTIVE_MASK_COUNT * sizeof(uint64_t)
		};

		template <typename I>
		struct partition_info_type
		{
			I begin;
			I end;

			constexpr auto size() const { return std::distance(begin, end); }
		};

		template <typename T>
		struct alignas(sizeof(size_t) * 2) partition_info_type<T*> //SIMD optimization for pointers, maybe?
		{
			T* begin;
			T* end;

			constexpr auto size() const { return std::distance(begin, end); }
		};

		template <typename T>
		constexpr void bit_set(T& mask, uint_fast8_t index) { mask |= ((T)1 << (T)index); }

		template <typename T>
		constexpr void bit_reset(T& mask, uint_fast8_t index) { mask &= (T)~((T)1 << (T)index); }

		template <typename I, typename F, typename D>
		constexpr void stripe_swap(partition_info_type<I>* partition_begin, const uint8_t* indirection, I begin, F& extract_digit, D digit_index)
		{
			using P = partition_info_type<I>;

			const P* const partition_end = partition_begin + RADIX_SIZE;
			P* const partitions = partition_begin;

			for (; partition_begin < partition_end - 1;)
			{
				P* const next_partition = partition_begin + 1;
				if (begin >= next_partition->begin)
				{
					partition_begin = next_partition;
					continue;
				}
				P* const target_partition = partitions + indirection[extract_digit(*begin, (uint)digit_index)];
				if (target_partition == partition_begin)
				{
					++begin;
					continue;
				}
				std::iter_swap(target_partition->end, begin);
				++target_partition->end;
			}
		}

		alignas(RADIX_SIZE)			inline thread_local uint8_t indirection[RADIX_SIZE];
		alignas(ACTIVE_MASK_SIZE)	inline thread_local uint64_t active_partitions[ACTIVE_MASK_COUNT];

		template <typename K, typename I, typename F, typename D>
		void stripe_core(I begin, I end, D digit_index, F& extract_digit)
		{
			assert(begin != end);
			using P = partition_info_type<I>;

			static thread_local P partitions[RADIX_SIZE];
			static thread_local K counts[RADIX_SIZE];

			I i;
			uint_fast16_t active_partition_count;

			while (true)
			{
				active_partition_count = 0;
				i = begin;

				(void)memset(counts, 0, sizeof(counts));
				(void)memset(active_partitions, 0, sizeof(active_partitions));

				for (; i < end; ++i)
				{
					const uint8_t digit = (uint8_t)extract_digit(*i, (uint)digit_index);
					assert(digit < RADIX_SIZE);
					++counts[digit];
					bit_set(active_partitions[digit >> 6], digit & 63);
				}

				for (uint_fast64_t mask : active_partitions)
					active_partition_count += (uint_fast16_t)_mm_popcnt_u64(mask);

				assert(active_partition_count != 0);

				if (active_partition_count > 1)
					break;
				if (digit_index == 0)
					return;
				--digit_index;
			}

			i = begin;
			active_partition_count = 0;

			for (uint_fast8_t mask_index = 0; mask_index < ACTIVE_MASK_COUNT; ++mask_index)
			{
				const uint_fast64_t mask_index_mask = (uint_fast64_t)mask_index << 6ui64;
				for (uint_fast64_t mask = active_partitions[mask_index]; mask != 0;)
				{
					const uint_fast8_t bit_index = (uint_fast8_t)_tzcnt_u64(mask); //find first set bit
					const uint_fast16_t index = (uint_fast16_t)(mask_index_mask | bit_index);
					const K k = counts[index];
					indirection[index] = active_partition_count;
					P& partition = partitions[active_partition_count];
					++active_partition_count;
					partition.begin = i;
					partition.end = i;
					i += k;
					bit_reset(mask, bit_index);
				}
			}

			i = begin;
			stripe_swap(partitions, indirection, begin, extract_digit, digit_index);

			if (digit_index == 0)
				return;
			--digit_index;

			i = begin;
			P remaining_partitions[RADIX_SIZE];
			for (uint_fast16_t partition_index = 0; partition_index < active_partition_count; ++partition_index)
			{
				const K k = counts[indirection[partition_index]];
				P& e = remaining_partitions[partition_index];
				e.begin = i;
				i += k;
				e.end = i;
			}

			if (active_partition_count > 128 && std::distance(begin, end) > 8192)
			{
				std::sort(remaining_partitions, remaining_partitions + active_partition_count, [](P& lhs, P& rhs) { return lhs.size() < rhs.size(); });
			}

			for (P* partition = remaining_partitions; partition < remaining_partitions + active_partition_count; ++partition)
			{
				stripe_core<K>(partition->begin, partition->end, digit_index, extract_digit);
			}
		}
	}

	template <typename I, typename F>
	void sort(I begin, I end, size_t max_digits, F&& extract_digit)
	{
		const auto size = std::distance(begin, end);
		--max_digits;

		if (size > std::numeric_limits<uint16_t>::max())
		{
			if (size > std::numeric_limits<uint32_t>::max())
				detail::stripe_core<uint64_t>(begin, end, max_digits, extract_digit);
			else
				detail::stripe_core<uint32_t>(begin, end, max_digits, extract_digit);
		}
		else
		{
			if (size > std::numeric_limits<uint8_t>::max())
				detail::stripe_core<uint16_t>(begin, end, max_digits, extract_digit);
			else
				detail::stripe_core<uint8_t>(begin, end, max_digits, extract_digit);
		}
	}
}

void custom_radix_sort(main_array& array)
{
	stripe_sort::sort(array.begin(), array.end(), item::max_radix(), [](item& e, uint radix)
	{
		return (uint8_t)(e.value >> (radix * 8));
	});
}