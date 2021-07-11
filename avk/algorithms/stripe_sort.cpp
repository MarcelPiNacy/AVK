#include "all.h"

#include <algorithm>
#include <bitset>
#include <array>
#include <cassert>
#include <intrin.h>

template <typename T, size_t Radix>
struct LexicographicKeyView;

template <size_t Radix>
struct LexicographicKeyView<uint8_t, Radix>
{
	using ValueType = uint8_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<uint16_t, Radix>
{
	using ValueType = uint16_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<uint32_t, Radix>
{
	using ValueType = uint32_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<uint64_t, Radix>
{
	using ValueType = uint64_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<int8_t, Radix>
{
	using ValueType = int8_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<int16_t, Radix>
{
	using ValueType = uint16_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<int32_t, Radix>
{
	using ValueType = uint32_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<int64_t, Radix>
{
	using ValueType = uint64_t;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (value >> (digit_index << 3)) & (Radix - 1); }
	constexpr size_t Extract(size_t digit_index) const { return (value >> (digit_index << 3)) & (Radix - 1); }
};

template <size_t Radix>
struct LexicographicKeyView<item, Radix>
{
	using ValueType = item;

	const ValueType& value;

	constexpr LexicographicKeyView(ValueType& value) : value(value) { }
	LexicographicKeyView(const LexicographicKeyView&) = default;
	LexicographicKeyView& operator=(const LexicographicKeyView&) = default;
	~LexicographicKeyView() = default;

	static constexpr size_t Extract(ValueType& value, size_t digit_index) { return (Radix - 1) & (value.value >> (digit_index << 3)); }
	constexpr size_t Extract(size_t digit_index) const { return (Radix - 1) & (value.value >> (digit_index << 3)); }
};

template <typename T, typename I, size_t Radix, size_t MaxRange, typename KeyExtractor = LexicographicKeyView<T, Radix>>
struct InplaceCountingSorter
{

	template <typename F>
	void Sort(I begin, I end, F&& extract)
	{
	}
};

template <typename T, typename I, size_t Radix = 256, typename KeyExtractor = LexicographicKeyView<T, Radix>>
struct Sorter
{
	std::array<I, Radix> partition_heads;
	std::array<I, Radix> partition_tails;

	constexpr void InitPartitions(
		I begin,
		size_t total_active_partition_count,
		const std::bitset<Radix>& presence,
		const std::array<size_t, Radix>& counts)
	{
		size_t this_partition = 0;
		size_t finished_partition_count = 0;
		while (true)
		{
			if (presence.test(this_partition))
			{
				partition_heads[this_partition] = begin;
				begin += counts[this_partition];
				partition_tails[this_partition] = begin;
				++finished_partition_count;
				if (finished_partition_count == total_active_partition_count)
					break;
			}
			++this_partition;
		}
	}

#ifdef __AVX__

	struct alignas(__m128i) DigitMap
	{
		union
		{
			__m128i xmmword;
			uint8_t digits[16];
		};
		uint8_t count;

		bool Insert(uint8_t digit)
		{
			if (Contains(digit))
				return false;
			digits[count] = digit;
			++count;
			return true;
		}

		bool Contains(uint8_t digit) const
		{
			uint_fast16_t presence = (uint_fast16_t)_mm_movemask_epi8(_mm_cmpeq_epi8(xmmword, _mm_set1_epi8(digit)));
			presence &= (1U << count) - 1U;
			return presence;
		}
	};

	void PermutePartitionsSIMD(
		I begin,
		size_t digit_index,
		size_t total_active_partition_count,
		const std::bitset<Radix>& presence)
	{
		I i = begin;
		size_t this_partition = 0;
		size_t finished_partition_count = 0;
		while (true)
		{
			while (!presence.test(this_partition))
				++this_partition;
			assert(this_partition < Radix);
			while (partition_heads[this_partition] != partition_tails[this_partition])
			{
				size_t digit = KeyExtractor::Extract(*partition_heads[this_partition], digit_index);
				if (digit == this_partition)
				{
					++partition_heads[this_partition];
					continue;
				}
				std::iter_swap(partition_heads[this_partition], partition_heads[digit]);
				++partition_heads[digit];
			}
			++finished_partition_count;
			if (finished_partition_count == total_active_partition_count)
				break;
			++this_partition;
		}
	}
#endif

	constexpr void PermutePartitionsDefault(
		I begin,
		size_t digit_index,
		size_t total_active_partition_count,
		const std::bitset<Radix>& presence)
	{
		I i = begin;
		size_t this_partition = 0;
		size_t finished_partition_count = 0;
		while (true)
		{
			while (!presence.test(this_partition))
				++this_partition;
			assert(this_partition < Radix);
			while (partition_heads[this_partition] != partition_tails[this_partition])
			{
				size_t digit = KeyExtractor::Extract(*partition_heads[this_partition], digit_index);
				if (digit == this_partition)
				{
					++partition_heads[this_partition];
					continue;
				}
				std::iter_swap(partition_heads[this_partition], partition_heads[digit]);
				++partition_heads[digit];
			}
			++finished_partition_count;
			if (finished_partition_count == total_active_partition_count)
				break;
			++this_partition;
		}
	}

	constexpr void SortCore(I begin, I end, size_t digit_index)
	{
		size_t total_active_partition_count = 0;
		std::bitset<Radix> presence = {};
		std::array<size_t, Radix> counts = {};

		while (true)
		{
			for (I i = begin; i != end; ++i)
			{
				size_t digit = KeyExtractor::Extract(*i, digit_index);
				presence.set(digit);
				++counts[digit];
			}
			total_active_partition_count = presence.count();
			if (total_active_partition_count != 1)
				break;
			if (digit_index == 0)
				return;
			--digit_index;
			counts = {};
			presence = {};
		}

		InitPartitions(begin, total_active_partition_count, presence, counts);

#if defined(__AVX__) || defined(__SSE__)
		PermutePartitionsSIMD(begin, digit_index, total_active_partition_count, presence);
#else
		PermutePartitionsDefault(begin, digit_index, total_active_partition_count, presence);
#endif

		if (digit_index == 0)
			return;
		--digit_index;

		size_t this_partition = 0;
		size_t finished_partition_count = 0;
		for (;; ++this_partition)
		{
			if (!presence.test(this_partition))
				continue;
			if (counts[this_partition] <= 1)
				continue;
			I i = begin;
			begin += counts[this_partition];
			SortCore(i, begin, digit_index);
			++finished_partition_count;
			if (finished_partition_count == total_active_partition_count)
				break;
		}
	}

	constexpr void Sort(I begin, I end, size_t digit_count)
	{
		if (std::distance(begin, end) > 2)
			SortCore(begin, end, digit_count - 1);
	}
};



void stripe_sort(main_array array)
{
	Sorter<item, item*, 256> sorter = {};
	sorter.Sort(array.begin(), array.end(), 4);
}