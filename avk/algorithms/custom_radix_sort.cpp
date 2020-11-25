#include "all.h"
#include "../internal/enforce.h"
#include <iterator>

namespace detail::stripe_sort
{
	template <typename I>
	void stripe_sort_core(I begin, I end, size_t digit_index)
	{
		enforce(begin < end);

		struct partition_info
		{
			union
			{
				size_t	count;
				I		begin;
			};

			I end;
		};

		uint64_t active_partitions[4] = {};
		partition_info partitions[256] = {};

		I i = begin;
		for (; i < end; ++i)
		{
			const uint8_t digit = extract_radix(*i, digit_index);
			(void)_bittestandset64((long long*)&active_partitions[digit >> 6], digit & 63);
			++partitions[digit].count;
		}

		const auto active_partition_count =
			_mm_popcnt_u32(active_partitions[0]) +
			_mm_popcnt_u32(active_partitions[1]) +
			_mm_popcnt_u32(active_partitions[2]) +
			_mm_popcnt_u32(active_partitions[3]);

		enforce(active_partition_count > 0);
		enforce(active_partition_count <= 256);

		if (active_partition_count > 1)
		{
			i = begin;
			for (uint8_t mask_index = 0; mask_index < 4; ++mask_index)
			{
				uint64_t mask = active_partitions[mask_index];
				while (mask != 0)
				{
					unsigned long bit_index;
					(void)_BitScanForward64(&bit_index, mask);
					const auto index = (mask_index << 6) | bit_index;
					partition_info& target_partition = partitions[index];
					const auto count = target_partition.count;
					target_partition.begin = i;
					i += count;
					mask &= ~(1ui64 << bit_index);
				}
			}

			i = begin;
			for (partition_info* partition = partitions; partition < partitions + 255;)
			{
				if (i >= partition[1].begin)
				{
					++partition;
					continue;
				}

				partition_info& target_partition = partitions[extract_radix(*i, digit_index)];
				if (&target_partition == partition)
				{
					++i;
					continue;
				}

				std::iter_swap(i, target_partition.end);
				++partition->end;
			}
		}

		if (digit_index > 0)
		{
			--digit_index;
			for (uint8_t mask_index = 0; mask_index < 4; ++mask_index)
			{
				uint64_t mask = active_partitions[mask_index];
				while (mask != 0)
				{
					unsigned long bit_index;
					(void)_BitScanForward64(&bit_index, mask);
					const auto index = (mask_index << 6) | bit_index;
					partition_info& partition = partitions[(mask_index << 6) | bit_index];
					stripe_sort_core(partition.begin, partition.end, digit_index);
					mask &= ~(1ui64 << bit_index);
				}
			}
		}
	}
}

void custom_radix_sort(main_array& array)
{
	detail::stripe_sort::stripe_sort_core(array.begin(), array.end(), item::max_radix() - 1);
}