#pragma once
#include "common.h"
#include "debug.h"
#include <iterator>
#include <chrono>


struct main_array;



namespace sort_stats
{
	void clear();
	void add_read(size_t count = 1);
	void add_write(size_t count = 1);
	void add_comparisson(size_t count = 1);
	void add_swap(size_t count = 1);
	void add_reversal(size_t count = 1);
	void add_memory_allocation(size_t count = 1);
	void add_memory_deallocation(size_t count = 1);
	size_t read_count();
	size_t write_count();
	size_t comparisson_count();
	size_t swap_count();
	size_t reversal_count();
	size_t memory_allocation_count();
	size_t memory_deallocation_count();
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
	uint32_t			value;
	uint32_t			original_position;
	item_color			color;
	mutable uint32_t	flags;

	item& operator=(const item& other);
	bool operator==(const item& other) const;
	bool operator!=(const item& other) const;
	bool operator<(const item& other) const;
	bool operator>(const item& other) const;
	bool operator<=(const item& other) const;
	bool operator>=(const item& other) const;

	static size_t max_radix(size_t base = 256);
};



struct item_iterator : std::random_access_iterator_tag
{
	using difference_type = ptrdiff_t;

	size_t index;

	constexpr item_iterator& operator=(const item_iterator& other) { index = other.index; return *this; }
	inline bool operator==(const item_iterator& other)	const { sort_stats::add_comparisson(); return index == other.index; }
	inline bool operator!=(const item_iterator& other)	const { sort_stats::add_comparisson(); return index != other.index; }
	inline bool operator<(const item_iterator& other)	const { sort_stats::add_comparisson(); return index <  other.index; }
	inline bool operator>(const item_iterator& other)	const { sort_stats::add_comparisson(); return index >  other.index; }
	inline bool operator<=(const item_iterator& other)	const { sort_stats::add_comparisson(); return index <= other.index; }
	inline bool operator>=(const item_iterator& other)	const { sort_stats::add_comparisson(); return index >= other.index; }

	constexpr item_iterator& operator++() { ++index; return *this; }
	constexpr item_iterator& operator--() { --index; return *this; }
	constexpr item_iterator operator++(int) { item_iterator r = *this; ++index; return r; }
	constexpr item_iterator operator--(int) { item_iterator r = *this; --index; return r; }
	constexpr item_iterator& operator+=(size_t offset) { index += offset; return *this; }
	constexpr item_iterator& operator-=(size_t offset) { index -= offset; return *this; }
	constexpr item_iterator operator+(size_t offset) const { item_iterator r = *this; r.index += offset; return r; }
	constexpr item_iterator operator-(size_t offset) const { item_iterator r = *this; r.index -= offset; return r; }
	constexpr item_iterator& operator+=(ptrdiff_t offset) { index += offset; return *this; }
	constexpr item_iterator& operator-=(ptrdiff_t offset) { index -= offset; return *this; }
	constexpr item_iterator operator+(ptrdiff_t offset) const { item_iterator r = *this; r.index += offset; return r; }
	constexpr item_iterator operator-(ptrdiff_t offset) const { item_iterator r = *this; r.index -= offset; return r; }
	constexpr ptrdiff_t operator-(const item_iterator& other) const { return index - other.index; }

	item& operator*() const;
};

namespace std
{
	constexpr auto distance(item_iterator begin, item_iterator end)
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



ptrdiff_t compare(const item& left, const item& right);
ptrdiff_t compare(main_array array, size_t left_index, size_t right_index);
void swap(item& left, item& right);
void swap(main_array array, size_t left_index, size_t right_index);
bool compare_swap(item& left, item& right);
bool compare_swap(main_array array, size_t left_index, size_t right_index);
void reverse(main_array array, size_t offset, size_t size);
uint8_t extract_byte(const item& value, size_t byte_index);
size_t extract_radix(const item& value, size_t radix_index, size_t radix = 256);



struct main_array
{
	using nanoseconds = std::chrono::nanoseconds;

	static bool resize(uint32_t size);
	static void finalize();

	static item& get(size_t index);
	item& operator[](size_t index) const;
	static size_t size();

	static item* begin();
	static item* end();

	static item* data();

	template <typename F>
	static void for_each(F&& function)
	{
		for (uint32_t i = 0; i < size(); ++i)
			function(begin()[i], i);
	}

	static void set_read_delay(int64_t nanoseconds);
	static void set_read_delay(nanoseconds ns);
	static void set_write_delay(int64_t nanoseconds);
	static void set_write_delay(nanoseconds ns);
	static void set_compare_delay(int64_t nanoseconds);
	static void set_compare_delay(nanoseconds ns);
	static void sleep(int64_t nanoseconds);
	static void sleep(nanoseconds ns);
};





struct scoped_highlight
{
	uint32_t& flags;

	scoped_highlight(item& e);
	scoped_highlight(uint32_t& flags);
	~scoped_highlight();
};