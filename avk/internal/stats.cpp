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

void non_atomic_store(shared_counter& where, uint value)
{
	*(uint*)&where = value;
}

namespace sort_stats
{
	void clear()
	{
		non_atomic_store(reads, 0);
		non_atomic_store(writes, 0);
		non_atomic_store(comparissons, 0);
		non_atomic_store(swaps, 0);
		non_atomic_store(reversals, 0);
		non_atomic_store(memory_allocations, 0);
		non_atomic_store(memory_deallocations, 0);
	}

	void add_read(uint count)
	{
		(void)reads.fetch_add(count, std::memory_order_release);
	}

	void add_write(uint count)
	{
		(void)writes.fetch_add(count, std::memory_order_release);
	}

	void add_comparisson(uint count)
	{
		(void)comparissons.fetch_add(count, std::memory_order_release);
	}

	void add_swap(uint count)
	{
		(void)swaps.fetch_add(count, std::memory_order_release);
	}

	void add_reversal(uint count)
	{
		(void)reversals.fetch_add(count, std::memory_order_release);
	}

	void add_memory_allocation(uint count)
	{
		(void)memory_allocations.fetch_add(count, std::memory_order_release);
	}

	void add_memory_deallocation(uint count)
	{
		(void)memory_deallocations.fetch_add(count, std::memory_order_release);
	}

	uint read_count()
	{
		return reads.load(std::memory_order_release);
	}

	uint write_count()
	{
		return writes.load(std::memory_order_release);
	}

	uint comparisson_count()
	{
		return comparissons.load(std::memory_order_release);
	}

	uint swap_count()
	{
		return swaps.load(std::memory_order_release);
	}

	uint reversal_count()
	{
		return reversals.load(std::memory_order_release);
	}

	uint memory_allocation_count()
	{
		return memory_allocations.load(std::memory_order_release);
	}

	uint memory_deallocation_count()
	{
		return memory_deallocations.load(std::memory_order_release);
	}
}