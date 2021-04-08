#include <atomic>
#include "common.h"
using std::atomic;
using shared_counter = atomic<size_t>;

static shared_counter reads;
static shared_counter writes;
static shared_counter comparissons;
static shared_counter swaps;
static shared_counter reversals;
static shared_counter memory_allocations;
static shared_counter memory_deallocations;

void non_atomic_store(shared_counter& where, size_t value)
{
	*(size_t*)&where = value;
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

	void add_read(size_t count)
	{
		(void)reads.fetch_add(count, std::memory_order_release);
	}

	void add_write(size_t count)
	{
		(void)writes.fetch_add(count, std::memory_order_release);
	}

	void add_comparisson(size_t count)
	{
		(void)comparissons.fetch_add(count, std::memory_order_release);
	}

	void add_swap(size_t count)
	{
		(void)swaps.fetch_add(count, std::memory_order_release);
	}

	void add_reversal(size_t count)
	{
		(void)reversals.fetch_add(count, std::memory_order_release);
	}

	void add_memory_allocation(size_t count)
	{
		(void)memory_allocations.fetch_add(count, std::memory_order_release);
	}

	void add_memory_deallocation(size_t count)
	{
		(void)memory_deallocations.fetch_add(count, std::memory_order_release);
	}

	size_t read_count()
	{
		return reads.load(std::memory_order_acquire);
	}

	size_t write_count()
	{
		return writes.load(std::memory_order_acquire);
	}

	size_t comparisson_count()
	{
		return comparissons.load(std::memory_order_acquire);
	}

	size_t swap_count()
	{
		return swaps.load(std::memory_order_acquire);
	}

	size_t reversal_count()
	{
		return reversals.load(std::memory_order_acquire);
	}

	size_t memory_allocation_count()
	{
		return memory_allocations.load(std::memory_order_acquire);
	}

	size_t memory_deallocation_count()
	{
		return memory_deallocations.load(std::memory_order_acquire);
	}
}