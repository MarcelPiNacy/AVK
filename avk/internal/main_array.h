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



struct item_color
{
	float r, g, b;
	
	static constexpr item_color black()	{ return{ 0, 0, 0 }; }
	static constexpr item_color white()	{ return{ 1, 1, 1 }; }
	static constexpr item_color red()	{ return{ 1, 0, 0 }; }
	static constexpr item_color green()	{ return{ 0, 1, 0 }; }
	static constexpr item_color blue()	{ return{ 0, 0, 1 }; }
};



struct item
{
	uint32_t	value;
	uint32_t	original_position;
	item_color	color;

	item& operator=(const item& other) noexcept;
	bool operator==(const item& other) const noexcept;
	bool operator!=(const item& other) const noexcept;
	bool operator<(const item& other) const noexcept;
	bool operator>(const item& other) const noexcept;
	bool operator<=(const item& other) const noexcept;
	bool operator>=(const item& other) const noexcept;

	static uint max_radix(uint radix = 256) noexcept;
};



struct item_raw
{
	uint32_t	value;
	uint32_t	original_position;
	item_color	color;
};



sint compare(const item& left, const item& right) noexcept;
void swap(item& left, item& right) noexcept;
void reverse(main_array& array, uint offset, uint size) noexcept;
uint extract_radix(const item& value, uint radix_index, uint radix = 256) noexcept;



struct main_array
{
	static void resize(uint32_t size) noexcept;
	static void finalize() noexcept;

	item& operator[](uint index) noexcept;
	static uint size() noexcept;

	static item* begin() noexcept;
	static item* end() noexcept;

	template <typename F>
	static void for_each(F&& function) noexcept
	{
		for (uint32_t i = 0; i < size(); ++i)
			function(begin()[i], i);
	}

	static void set_read_delay(double seconds);
	static void set_write_delay(double seconds);
	static void set_compare_delay(double seconds);
	static void sleep(double seconds);
};