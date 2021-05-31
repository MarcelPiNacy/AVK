#include "all.h"
#include "../internal/parallel_for.h"
#include <vector>
#include <bitset>
#include <atomic>
#include <cassert>



static constexpr auto BREADTH_SORT_RADIX_SIZE = 256;
static constexpr auto BREADTH_SORT_SEQUENTIAL_THRESHOLD = 128;

template <typename I, typename J>
static void parallel_move(I begin, I end, J out_begin)
{
	parallel_for(begin, end, [&](I e)
	{
		*std::next(out_begin, std::distance(begin, e)) = std::move(*e);
	});
}

template <bool MustMoveBack = true, typename I, typename J, typename F>
static void breadth_sort_pass_sequential(I begin, I end, J buffer_begin, J buffer_end, size_t digit_index, F&& extract)
{

	std::bitset<BREADTH_SORT_RADIX_SIZE> presence;
	size_t counts[BREADTH_SORT_RADIX_SIZE];

	while (true)
	{
		presence = {};
		memset(counts, 0, sizeof(counts));

		--digit_index;

		for (I i = begin; i != end; ++i)
		{
			size_t digit = extract(*i, digit_index);
			presence.set(digit);
			++counts[digit];
		}

		if (presence.count() != 1)
			break;
		if (digit_index == 0)
			return;
	}
	
	size_t offsets[BREADTH_SORT_RADIX_SIZE];
	size_t offset = 0;

	for (size_t i = 0; i != BREADTH_SORT_RADIX_SIZE; ++i)
	{
		offsets[i] = offset;
		offset += counts[i];
	}

	for (I i = begin; i != end; ++i)
	{
		size_t digit = extract(*i, digit_index);
		size_t& index = offsets[digit];
		*std::next(buffer_begin, index) = std::move(*i);
		++index;
	}

	if (digit_index == 0)
	{
		if constexpr (MustMoveBack)
		{
			if (std::distance(begin, end) > BREADTH_SORT_RADIX_SIZE)
				parallel_move(buffer_begin, buffer_end, begin);
			else
				std::move(buffer_begin, buffer_end, begin);
		}
		return;
	}

	offset = 0;
	for (size_t i = 0; i != BREADTH_SORT_RADIX_SIZE; ++i)
	{
		if (counts[i] == 0)
			continue;
		size_t size = counts[i];
		auto new_begin = buffer_begin + offset;
		auto new_end = new_begin + size;
		auto new_buffer_begin = begin + offset;
		auto new_buffer_end = new_buffer_begin + size;
		breadth_sort_pass_sequential<!MustMoveBack>(new_begin, new_end, new_buffer_begin, new_buffer_end, digit_index, extract);
		offset += size;
	}
}

template <bool MustMoveBack = true, typename I, typename J, typename F>
static void breadth_sort_pass_parallel(I begin, I end, J buffer_begin, J buffer_end, size_t digit_index, F&& extract)
{
#ifdef _DEBUG
	size_t range = std::distance(begin, end);
#endif

	std::atomic<uint64_t> presence[std::max(1, BREADTH_SORT_RADIX_SIZE / 64)];
	std::atomic<size_t> counts[BREADTH_SORT_RADIX_SIZE];

	while (true)
	{
		--digit_index;

		memset(presence, 0, sizeof(presence));
		memset(counts, 0, sizeof(counts));

		parallel_for(begin, end, [&](I e)
		{
			size_t digit = extract(*e, digit_index);
			size_t mask_index = digit >> 6;
			size_t bit_index = digit & 63;
			(void)presence[mask_index].fetch_or(1UI64 << bit_index, std::memory_order_release);
			(void)counts[digit].fetch_add(1, std::memory_order_acquire);
		});

		uint_fast16_t popcount =
			(uint_fast16_t)__popcnt64(presence[0].load(std::memory_order_acquire)) +
			(uint_fast16_t)__popcnt64(presence[1].load(std::memory_order_acquire)) +
			(uint_fast16_t)__popcnt64(presence[2].load(std::memory_order_acquire)) +
			(uint_fast16_t)__popcnt64(presence[3].load(std::memory_order_acquire));

		if (popcount != 1)
			break;

		if (digit_index == 0)
			return;
	}

	size_t offset = 0;
	size_t offsets[BREADTH_SORT_RADIX_SIZE] = {};
	std::atomic<size_t> shared_offsets[BREADTH_SORT_RADIX_SIZE];

	for (size_t i = 0; i != BREADTH_SORT_RADIX_SIZE; ++i)
	{
		offsets[i] = offset;
		offset += counts[i].load(std::memory_order_acquire);
	}

	memcpy((void*)shared_offsets, (void*)offsets, sizeof(offsets));

	parallel_for(begin, end, [&](I e)
	{
		size_t digit = extract(*e, digit_index);
		size_t index = shared_offsets[digit].fetch_add(1, std::memory_order_acquire);
		assert(index < range);
		*std::next(buffer_begin, index) = std::move(*e);
	});

	if (digit_index == 0)
	{
		if constexpr (MustMoveBack)
		{
			if (std::distance(begin, end) > BREADTH_SORT_SEQUENTIAL_THRESHOLD)
				parallel_move(buffer_begin, buffer_end, begin);
			else
				std::move(buffer_begin, buffer_end, begin);
		}
		return;
	}

	parallel_for<size_t>(0, BREADTH_SORT_RADIX_SIZE, [&](size_t i)
	{
		size_t size = counts[i].load(std::memory_order_acquire);
		if (size == 0)
			return;

		auto new_begin = buffer_begin + offsets[i];
		auto new_end = new_begin + size;

		auto new_buffer_begin = begin + offsets[i];
		auto new_buffer_end = new_buffer_begin + size;

		if (size > 4096)
			breadth_sort_pass_parallel<!MustMoveBack>(new_begin, new_end, new_buffer_begin, new_buffer_end, digit_index, extract);
		else
			breadth_sort_pass_sequential<!MustMoveBack>(new_begin, new_end, new_buffer_begin, new_buffer_end, digit_index, extract);
	});
}



void breadth_sort(main_array array)
{
	array.mark_as_parallel_sort();
	std::vector<item> buffer;
	buffer.resize(array.size());

	breadth_sort_pass_parallel(
		array.begin(), array.end(),
		buffer.begin(), buffer.end(), item::max_radix(BREADTH_SORT_RADIX_SIZE), [](const item& e, size_t index) { return extract_radix(e, (size_t)index, BREADTH_SORT_RADIX_SIZE); });
}