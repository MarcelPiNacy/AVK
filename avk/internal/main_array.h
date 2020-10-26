#pragma once
#include "common.h"



struct main_array;



/// <summary>
/// Used to keep sort statistics. All functions EXCEPT clear are thread-safe.
/// </summary>
namespace stats
{
	void clear() noexcept;

	void add_read(uint count = 1) noexcept;
	void add_write(uint count = 1) noexcept;
	void add_comparisson(uint count = 1) noexcept;
	void add_swap(uint count = 1) noexcept;
	void add_reversal(uint count = 1) noexcept;
	void add_memory_allocation(uint count = 1) noexcept;
	void add_memory_deallocation(uint count = 1) noexcept;

	uint read_count() noexcept;
	uint write_count() noexcept;
	uint comparisson_count() noexcept;
	uint swap_count() noexcept;
	uint reversal_count() noexcept;
	uint memory_allocation_count() noexcept;
	uint memory_deallocation_count() noexcept;
}



struct alignas(sizeof(uint64_t)) element
{
	uint32_t	internal_value;
	uint32_t	initial_position;

	operator uint() const noexcept;

	element& operator=(const element& other) noexcept;
	bool operator==(const element& other) const noexcept;
	bool operator!=(const element& other) const noexcept;
	bool operator<(const element& other) const noexcept;
	bool operator>(const element& other) const noexcept;
	bool operator<=(const element& other) const noexcept;
	bool operator>=(const element& other) const noexcept;
};



sint compare(const element& left, const element& right) noexcept;
void swap(element& left, element& right) noexcept;
void reverse(main_array& array, uint offset, uint size);



struct main_array
{
	/// <summary>
	/// Initializes the main array.
	/// </summary>
	/// <param name="size">
	/// The size of the array.
	/// </param>
	static void resize(uint size) noexcept;
	static void finalize() noexcept;

	element& operator[](uint index) noexcept;
	static uint size() noexcept;

	static element* begin() noexcept;
	static element* end() noexcept;

	template <typename F>
	static void fill(F&& function) noexcept
	{
		for (uint i = 0; i < size(); ++i)
			function(begin()[i], i);
	}

	static void set_read_delay(uint32_t milliseconds);
	static void set_write_delay(uint32_t milliseconds);
	static void set_compare_delay(uint32_t milliseconds);
	static void sleep(uint32_t milliseconds);

};