#pragma once
#include "common.h"
#include "debug.h"
#include <iterator>
#include <chrono>


struct main_array;



namespace sort_stats
{
	void clear();
	void add_read(uint count = 1);
	void add_write(uint count = 1);
	void add_comparisson(uint count = 1);
	void add_swap(uint count = 1);
	void add_reversal(uint count = 1);
	void add_memory_allocation(uint count = 1);
	void add_memory_deallocation(uint count = 1);
	uint read_count();
	uint write_count();
	uint comparisson_count();
	uint swap_count();
	uint reversal_count();
	uint memory_allocation_count();
	uint memory_deallocation_count();
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

	static uint max_radix(uint base = 256);
};



struct item_iterator : std::random_access_iterator_tag
{
	using difference_type = sint;

	uint index;

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
	constexpr item_iterator& operator+=(uint offset) { index += offset; return *this; }
	constexpr item_iterator& operator-=(uint offset) { index -= offset; return *this; }
	constexpr item_iterator operator+(uint offset) const { item_iterator r = *this; r.index += offset; return r; }
	constexpr item_iterator operator-(uint offset) const { item_iterator r = *this; r.index -= offset; return r; }
	constexpr item_iterator& operator+=(sint offset) { index += offset; return *this; }
	constexpr item_iterator& operator-=(sint offset) { index -= offset; return *this; }
	constexpr item_iterator operator+(sint offset) const { item_iterator r = *this; r.index += offset; return r; }
	constexpr item_iterator operator-(sint offset) const { item_iterator r = *this; r.index -= offset; return r; }
	constexpr sint operator-(const item_iterator& other) const { return index - other.index; }

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



sint compare(const item& left, const item& right);
sint compare(main_array array, uint left_index, uint right_index);
void swap(item& left, item& right);
void swap(main_array array, uint left_index, uint right_index);
bool compare_swap(item& left, item& right);
bool compare_swap(main_array array, uint left_index, uint right_index);
void reverse(main_array array, uint offset, uint size);
uint8_t extract_byte(const item& value, uint byte_index);
uint extract_radix(const item& value, uint radix_index, uint radix = 256);



struct main_array
{
	using nanoseconds = std::chrono::nanoseconds;

	static bool resize(uint32_t size);
	static void finalize();

	static item& get(uint index);
	item& operator[](uint index) const;
	static uint size();

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



#include <cmts.h>
#include "platform.h"
#include <cassert>
#include <thread>

template <typename F>
void as_parallel(F&& function)
{
	cmts_result_t code;
	cmts_ext_debugger_init_options_t debugger_options = {};
	debugger_options.ext_type = CMTS_EXT_TYPE_DEBUGGER;
	debugger_options.message_callback = [](void* context, const cmts_ext_debugger_message_t* message)
	{
		OutputDebugStringA(message->message);
		OutputDebugStringA("\n");
	};
	cmts_init_options_t init_options = {};
	init_options.ext = &debugger_options;
	init_options.flags = CMTS_INIT_FLAGS_USE_AFFINITY;
	init_options.thread_count = cmts_processor_count();
	init_options.max_tasks = std::min(main_array::size(), CMTS_MAX_TASKS);
	init_options.task_stack_size = cmts_default_task_stack_size();
	code = cmts_lib_init(&init_options);
	AVK_ASSERT(cmts_ext_debugger_enabled());
	AVK_ASSERT(code == CMTS_OK);
	F fn = std::forward<F>(function);
	cmts_dispatch_options_t options = {};
	options.parameter = &fn;
	options.flags = CMTS_DISPATCH_FLAGS_FORCE;
	code = cmts_dispatch([](void* ptr)
	{
		(*(F*)ptr)();
		cmts_lib_exit_signal();
	}, &options);
	AVK_ASSERT(code == CMTS_OK);
	code = cmts_lib_exit_await(nullptr);
	AVK_ASSERT(code == CMTS_OK);
}

template <typename I, typename F>
void parallel_for(I begin, I end, F&& body)
{
	if (begin == end)
		return;
	AVK_ASSERT(begin < end);
	AVK_ASSERT(cmts_is_task());
	size_t count;
	if constexpr (std::is_integral_v<I>)
	{
		if constexpr (std::is_signed_v<I>)
			count = end - begin;
		else
			count = (size_t)end - (size_t)begin;
	}
	else
	{
		count = std::distance(begin, end);
	}

	AVK_ASSERT(count < UINT32_MAX);

	cmts_result_t code;
	struct wrapper_type
	{
		F function;
		I current;
		cmts_barrier_t barrier;
	};
	wrapper_type wrapper = { std::forward<F>(body), begin };
	cmts_counter_t counter;
	cmts_counter_init(&counter, count);
	cmts_dispatch_options_t options = {};
	options.parameter = &wrapper;
	options.flags = CMTS_DISPATCH_FLAGS_FORCE;
	options.sync_type = CMTS_SYNC_TYPE_COUNTER;
	options.sync_object = &counter;
	for (; begin != end; ++begin)
	{
		wrapper.current = begin;
		cmts_barrier_init(&wrapper.barrier);
		code = cmts_dispatch([](void* ptr)
		{
			wrapper_type& wrapper = *(wrapper_type*)ptr;
			I it = std::move(wrapper.current);
			cmts_barrier_signal(&wrapper.barrier);
			wrapper.function(it);
		}, &options);
		AVK_ASSERT(code == CMTS_OK);
		cmts_barrier_await(&wrapper.barrier);
	}
	code = cmts_counter_await(&counter);
}