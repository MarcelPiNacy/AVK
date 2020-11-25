#pragma once
#include "common.h"
#include <iterator>



struct main_array;



namespace sort_stats
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
	
	static constexpr item_color black()	noexcept { return{ 0, 0, 0 }; }
	static constexpr item_color white()	noexcept { return{ 1, 1, 1 }; }
	static constexpr item_color red()	noexcept { return{ 1, 0, 0 }; }
	static constexpr item_color green()	noexcept { return{ 0, 1, 0 }; }
	static constexpr item_color blue()	noexcept { return{ 0, 0, 1 }; }
};



struct item
{
	uint32_t			value;
	uint32_t			original_position;
	item_color			color;
	mutable uint32_t	flags;

	item& operator=(const item& other) noexcept;
	bool operator==(const item& other) const noexcept;
	bool operator!=(const item& other) const noexcept;
	bool operator<(const item& other) const noexcept;
	bool operator>(const item& other) const noexcept;
	bool operator<=(const item& other) const noexcept;
	bool operator>=(const item& other) const noexcept;

	static uint max_radix(uint radix = 256) noexcept;
};



struct item_iterator : std::random_access_iterator_tag
{
	using difference_type = sint;

	uint index;

	constexpr item_iterator& operator=(const item_iterator& other) noexcept { index = other.index; return *this; }
	constexpr bool operator==(const item_iterator& other)	const noexcept { return index == other.index; }
	constexpr bool operator!=(const item_iterator& other)	const noexcept { return index != other.index; }
	constexpr bool operator<(const item_iterator& other)	const noexcept { return index <  other.index; }
	constexpr bool operator>(const item_iterator& other)	const noexcept { return index >  other.index; }
	constexpr bool operator<=(const item_iterator& other)	const noexcept { return index <= other.index; }
	constexpr bool operator>=(const item_iterator& other)	const noexcept { return index >= other.index; }

	constexpr item_iterator& operator++() noexcept { ++index; return *this; }
	constexpr item_iterator& operator--() noexcept { --index; return *this; }
	constexpr item_iterator operator++(int) noexcept { item_iterator r = *this; ++index; return r; }
	constexpr item_iterator operator--(int) noexcept { item_iterator r = *this; --index; return r; }
	constexpr item_iterator& operator+=(uint offset) noexcept { index += offset; return *this; }
	constexpr item_iterator& operator-=(uint offset) noexcept { index -= offset; return *this; }
	constexpr item_iterator operator+(uint offset) const noexcept { item_iterator r = *this; r.index += offset; return r; }
	constexpr item_iterator operator-(uint offset) const noexcept { item_iterator r = *this; r.index -= offset; return r; }
	constexpr item_iterator& operator+=(sint offset) noexcept { index += offset; return *this; }
	constexpr item_iterator& operator-=(sint offset) noexcept { index -= offset; return *this; }
	constexpr item_iterator operator+(sint offset) const noexcept { item_iterator r = *this; r.index += offset; return r; }
	constexpr item_iterator operator-(sint offset) const noexcept { item_iterator r = *this; r.index -= offset; return r; }
	constexpr sint operator-(const item_iterator& other) const noexcept { return index - other.index; }

	item& operator*() const noexcept;
};

namespace std
{
	constexpr auto distance(item_iterator begin, item_iterator end) noexcept
	{
		return end - begin;
	}
}



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
	static void internal_lock() noexcept;
	static void internal_unlock() noexcept;

	static bool resize(uint32_t size) noexcept;
	static void finalize() noexcept;

	static item& get(uint index) noexcept;
	item& operator[](uint index) noexcept;
	static uint size() noexcept;

	static item* begin() noexcept;
	static item* end() noexcept;

	static item* data() noexcept;

	template <typename F>
	static void for_each(F&& function) noexcept
	{
		for (uint32_t i = 0; i < size(); ++i)
			function(begin()[i], i);
	}

	static void set_read_delay(double seconds) noexcept;
	static void set_write_delay(double seconds) noexcept;
	static void set_compare_delay(double seconds) noexcept;
	static void sleep(double seconds) noexcept;
};



struct scoped_highlight
{
	uint32_t& flags;

	inline scoped_highlight(uint32_t& flags) noexcept
		: flags(flags)
	{
		main_array::internal_lock();
		flags |= 1;
		main_array::internal_unlock();
	}

	inline ~scoped_highlight() noexcept
	{
		main_array::internal_lock();
		flags &= ~1U;
		main_array::internal_unlock();
	}
};