#include <atomic>
#include "common.h"
using std::atomic;
using shared_counter = atomic<uint>;

static shared_counter reads;
static shared_counter writes;
static shared_counter comparissons;
static shared_counter swaps;
static shared_counter reversals;
static shared_counter memory_allocations;
static shared_counter memory_deallocations;

void non_atomic_store(shared_counter& where, uint value) noexcept
{
	*(uint*)&where = value;
}

namespace stats
{
	void clear() noexcept
	{
		non_atomic_store(reads, 0);
		non_atomic_store(writes, 0);
		non_atomic_store(comparissons, 0);
		non_atomic_store(swaps, 0);
		non_atomic_store(reversals, 0);
		non_atomic_store(memory_allocations, 0);
		non_atomic_store(memory_deallocations, 0);
	}

	void add_read(uint count) noexcept
	{
		(void)reads.fetch_add(count, std::memory_order_release);
	}

	void add_write(uint count) noexcept
	{
		(void)writes.fetch_add(count, std::memory_order_release);
	}

	void add_comparisson(uint count) noexcept
	{
		(void)comparissons.fetch_add(count, std::memory_order_release);
	}

	void add_swap(uint count) noexcept
	{
		(void)swaps.fetch_add(count, std::memory_order_release);
	}

	void add_reversal(uint count) noexcept
	{
		(void)reversals.fetch_add(count, std::memory_order_release);
	}

	void add_memory_allocation(uint count) noexcept
	{
		(void)memory_allocations.fetch_add(count, std::memory_order_release);
	}

	void add_memory_deallocation(uint count) noexcept
	{
		(void)memory_deallocations.fetch_add(count, std::memory_order_release);
	}

	uint read_count() noexcept
	{
		return reads.load(std::memory_order_release);
	}

	uint write_count() noexcept
	{
		return writes.load(std::memory_order_release);
	}

	uint comparisson_count() noexcept
	{
		return comparissons.load(std::memory_order_release);
	}

	uint swap_count() noexcept
	{
		return swaps.load(std::memory_order_release);
	}

	uint reversal_count() noexcept
	{
		return reversals.load(std::memory_order_release);
	}

	uint memory_allocation_count() noexcept
	{
		return memory_allocations.load(std::memory_order_release);
	}

	uint memory_deallocation_count() noexcept
	{
		return memory_deallocations.load(std::memory_order_release);
	}
}