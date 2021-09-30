#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <chrono>
#include <thread>
#include "..\main_array.h"
#include <cassert>
#include "vulkan_state.h"
#include "../defer.h"
#include <shared_mutex>
#include <Comet.hpp>
using std::chrono::duration_cast;

using clock_type = std::chrono::steady_clock;
static std::chrono::nanoseconds read_delay;
static std::chrono::nanoseconds write_delay;
static std::chrono::nanoseconds compare_delay;

static uint32_t find_buffer_memory_type(VkBuffer buffer, VkMemoryPropertyFlags flags)
{
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(device, buffer, &req);
	uint32_t i = 0;
	for (; i < physical_device_properties.memoryTypeCount; ++i)
	{
		const bool a = (physical_device_properties.memoryTypes[i].propertyFlags & flags) == flags;
		const bool b = req.memoryTypeBits & (1 << i);
		if (a && b)
			return i;
	}
	return UINT32_MAX;
}

bool main_array::resize(uint32_t size)
{
	if (main_array_buffer != VK_NULL_HANDLE)
	{
		finalize();
	}

	main_array_size = size;
	main_array_capacity = round_pow2(size);
	size *= sizeof(item);
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = main_array_capacity * sizeof(item);
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(device, &buffer_info, nullptr, &main_array_buffer) != VK_SUCCESS)
		return false;
	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = buffer_info.size;
	alloc_info.memoryTypeIndex = find_buffer_memory_type(main_array_buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	if (alloc_info.memoryTypeIndex == UINT32_MAX)
		return false;
	if (vkAllocateMemory(device, &alloc_info, nullptr, &main_array_memory) != VK_SUCCESS)
		return false;
	if (vkBindBufferMemory(device, main_array_buffer, main_array_memory, 0) != VK_SUCCESS)
		return false;
	if (vkMapMemory(device, main_array_memory, 0, buffer_info.size, 0, (void**)&main_array_mapping) != VK_SUCCESS)
		return false;
	return true;
}

void main_array::finalize()
{
	vkUnmapMemory(device, main_array_memory);
	main_array_mapping = nullptr;
	vkFreeMemory(device, main_array_memory, nullptr);
	vkDestroyBuffer(device, main_array_buffer, nullptr);
	main_array_buffer = nullptr;
}

item& main_array::get(size_t index)
{
	AVK_ASSERT(main_array_mapping != nullptr);
	AVK_ASSERT(index < size());
	return main_array_mapping[index];
}

item& main_array::operator[](size_t index) const
{
	AVK_ASSERT(main_array_mapping != nullptr);
	AVK_ASSERT(index < size());
	return main_array_mapping[index];
}

size_t main_array::size()
{
	return main_array_size;
}

item* main_array::begin()
{
	return main_array_mapping;
}

item* main_array::end()
{
	return main_array_mapping + size();
}

item* main_array::data()
{
	return main_array_mapping;
}

void main_array::set_read_delay(int64_t ns)
{
	read_delay = std::chrono::nanoseconds(ns);
}

void main_array::set_read_delay(nanoseconds ns)
{
	read_delay = ns;
}

void main_array::set_write_delay(int64_t ns)
{
	write_delay = std::chrono::nanoseconds(ns);
}

void main_array::set_write_delay(nanoseconds ns)
{
	write_delay = ns;
}

void main_array::set_compare_delay(int64_t ns)
{
	compare_delay = std::chrono::nanoseconds(ns);
}

void main_array::set_compare_delay(nanoseconds ns)
{
	compare_delay = ns;
}

void main_array::sleep(int64_t ns)
{
	sleep(nanoseconds(ns));
}

static std::atomic_bool is_parallel;

void main_array::sleep(nanoseconds duration)
{
	using namespace std::chrono;
	if (duration.count() <= 50)
	{
		auto start = high_resolution_clock::now();
		while (high_resolution_clock::now() - start < duration)
			platform::yield_cpu();
	}
	else
	{
		if (is_parallel.load(std::memory_order_acquire))
		{
			auto start = high_resolution_clock::now();
			while (high_resolution_clock::now() - start < duration)
				Comet::Yield();
		}
		else
		{
			if (duration_cast<microseconds>(duration).count() < 1000)
			{
				auto start = high_resolution_clock::now();
				while (high_resolution_clock::now() - start < duration)
					SwitchToThread();
			}
			else
			{
				Sleep((DWORD)duration_cast<milliseconds>(duration).count());
			}
		}
	}
}

void main_array::begin_parallel_sort()
{
	is_parallel.store(true, std::memory_order_release);
}

void main_array::end_parallel_sort()
{
	is_parallel.store(false, std::memory_order_release);
}

item& item::operator=(const item& other)
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay + write_delay));
	sort_stats::add_read();
	sort_stats::add_write();
	value = other.value;
	color = other.color;
	return *this;
}

bool item::operator==(const item& other) const
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	return value == other.value;
}

bool item::operator!=(const item& other) const
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	return value != other.value;
}

bool item::operator<(const item& other) const
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	return value < other.value;
}

bool item::operator>(const item& other) const
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	return value > other.value;
}

bool item::operator<=(const item& other) const
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	return value <= other.value;
}

bool item::operator>=(const item& other) const
{
	scoped_highlight highlight(flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	return value >= other.value;
}

size_t item::max_radix(size_t radix)
{
	return 32 / floor_log2(radix);
}

ptrdiff_t compare(const item& left, const item& right)
{
	scoped_highlight highlight_left(left.flags);
	scoped_highlight highlight_right(right.flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2));
	sort_stats::add_comparisson();
	sort_stats::add_read(2);
	if (left.value == right.value)
		return 0;
	ptrdiff_t r = 1;
	if (left.value < right.value)
		r = -1;
	return r;
}

ptrdiff_t compare(main_array array, size_t left_index, size_t right_index)
{
	return compare(array[left_index], array[right_index]);
}

void swap(item& left, item& right)
{
	scoped_highlight highlight_left(left.flags);
	scoped_highlight highlight_right(right.flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay * 2 + write_delay * 2));
	sort_stats::add_swap();
	sort_stats::add_read(2);
	sort_stats::add_write(2);
	const auto v = left.value;
	const auto c = left.color;
	left.value = right.value;
	left.color = right.color;
	right.value = v;
	right.color = c;
}

void swap(main_array array, size_t left_index, size_t right_index)
{
	swap(array[left_index], array[right_index]);
}

bool compare_swap(item& left, item& right)
{
	const bool r = left > right;
	if (r)
		swap(left, right);
	return r;
}

bool compare_swap(main_array array, size_t left_index, size_t right_index)
{
	return compare_swap(array[left_index], array[right_index]);
}

void reverse(main_array array, size_t offset, size_t size)
{
	sort_stats::add_reversal(1);
	size_t begin = offset;
	size_t end = offset + size - 1;
	while (begin < end)
	{
		swap(array[begin], array[end]);
		++begin;
		--end;
	}
}

uint8_t extract_byte(const item& value, size_t byte_index)
{
	return (uint8_t)extract_radix(value, byte_index);
}

size_t extract_radix(const item& value, size_t radix_index, size_t radix)
{
	scoped_highlight highlight(value.flags);
	main_array::sleep(duration_cast<std::chrono::nanoseconds>(read_delay));
#if UINT32_MAX == UINTPTR_MAX
	AVK_ASSERT(__popcnt(radix) == 1);
#else
	AVK_ASSERT(__popcnt64(radix) == 1);
#endif
	const size_t log2 = floor_log2(radix);
	const size_t mask = radix - 1;
	return ((size_t)value.value >> (radix_index * log2)) & mask;
}

item& item_iterator::operator*() const
{
	return main_array::get(index);
}

scoped_highlight::scoped_highlight(item& e)
	: flags(e.flags)
{
	flags |= 1;
}

scoped_highlight::scoped_highlight(uint32_t& flags)
	: flags(flags)
{
	flags |= 1;
}

scoped_highlight::~scoped_highlight()
{
	flags &= ~1U;
}