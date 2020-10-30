#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include <chrono>
#include "..\main_array.h"
#include <cassert>
#include "../enforce.h"
#include "vulkan_state.h"
#include "../defer.h"
#include <mutex>
using std::scoped_lock;

static double read_delay;
static double write_delay;
static double compare_delay;

static uint32_t find_buffer_memory_type(VkBuffer buffer, VkMemoryPropertyFlags flags)
{
	VkMemoryRequirements req;
	vkGetBufferMemoryRequirements(device, buffer, &req);
	uint32_t i = 0;
	for (; i < physical_device_properties.memoryTypeCount; ++i)
	{
		const bool a = (physical_device_properties.memoryTypes[i].propertyFlags & flags) == flags;
		const bool b = req.memoryTypeBits & (1 << i);
		if (a & b)
			return i;
	}
	return UINT32_MAX;
}

static uint8_t fast_log2(size_t value)
{
#if UINTPTR_MAX == UINT32_MAX
	return (uint8_t)_lzcnt_u32(value);
#else
	return (uint8_t)_lzcnt_u64(value);
#endif
}

static uint32_t round_pow2(size_t value)
{
	return 1 << ((sizeof(size_t) * 8) - fast_log2(value));
}

void main_array::resize(uint32_t size) noexcept
{
	if (main_array_buffer != VK_NULL_HANDLE)
		finalize();
	main_array_size = size;
	main_array_capacity = round_pow2(size);
	VkBufferCreateInfo buffer_info = {};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = main_array_capacity * sizeof(item);
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	enforce(vkCreateBuffer(device, &buffer_info, nullptr, &main_array_buffer) == VK_SUCCESS);
	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = buffer_info.size;
	alloc_info.memoryTypeIndex = find_buffer_memory_type(main_array_buffer, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	enforce(vkAllocateMemory(device, &alloc_info, nullptr, &main_array_memory) == VK_SUCCESS);
	enforce(vkBindBufferMemory(device, main_array_buffer, main_array_memory, 0) == VK_SUCCESS);
	enforce(vkMapMemory(device, main_array_memory, 0, buffer_info.size, 0, (void**)&main_array_mapping) == VK_SUCCESS);
}

void main_array::finalize() noexcept
{
	vkUnmapMemory(device, main_array_memory);
	main_array_mapping = nullptr;
	vkFreeMemory(device, main_array_memory, nullptr);
	vkDestroyBuffer(device, main_array_buffer, nullptr);
	main_array_buffer = nullptr;
}

item& main_array::operator[](uint index) noexcept
{
	enforce(main_array_mapping != nullptr);
	enforce(index < size());
	return main_array_mapping[index];
}

uint main_array::size() noexcept
{
	return main_array_size;
}

item* main_array::begin() noexcept
{
	return main_array_mapping;
}

item* main_array::end() noexcept
{
	return main_array_mapping + main_array_size;
}

void main_array::set_read_delay(double seconds)
{
	read_delay = seconds;
}

void main_array::set_write_delay(double seconds)
{
	write_delay = seconds;
}

void main_array::set_compare_delay(double seconds)
{
	compare_delay = seconds;
}

void main_array::sleep(double seconds)
{
	constexpr double sleep_threshold = 1.0 / 1000000.0;
	if (seconds < sleep_threshold)
	{
		seconds *= 1000.0;
		Sleep((DWORD)seconds);
	}
	else
	{
		using namespace std::chrono;
		seconds *= 1000'000;
		const uint64_t nanoseconds = (uint64_t)seconds;
		const auto start = high_resolution_clock::now();
		while ((high_resolution_clock::now() - start).count() < nanoseconds)
			platform::yield_cpu();
	}
}

item& item::operator=(const item& other) noexcept
{
	main_array::sleep(read_delay + write_delay);
	stats::add_read();
	stats::add_write();
	scoped_lock guard(array_lock);
	value = other.value;
	color = other.color;
	return *this;
}

bool item::operator==(const item& other) const noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	return value == other.value;
}

bool item::operator!=(const item& other) const noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	return value != other.value;
}

bool item::operator<(const item& other) const noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	return value < other.value;
}

bool item::operator>(const item& other) const noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	return value > other.value;
}

bool item::operator<=(const item& other) const noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	return value <= other.value;
}

bool item::operator>=(const item& other) const noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	return value >= other.value;
}

sint compare(const item& left, const item& right) noexcept
{
	main_array::sleep(read_delay * 2);
	stats::add_comparisson();
	stats::add_read(2);
	scoped_lock guard(array_lock);
	if (left.value == right.value)
		return 0;
	sint r = 1;
	if (left.value < right.value)
		r = -1;
	return r;
}

void swap(item& left, item& right) noexcept
{
	main_array::sleep(read_delay * 2 + write_delay * 2);
	stats::add_swap();
	stats::add_read(2);
	stats::add_write(2);
	scoped_lock guard(array_lock);
	const auto v = left.value;
	const auto c = left.color;
	left.value = right.value;
	left.color = right.color;
	right.value = v;
	right.color = c;
}

void reverse(main_array& array, uint offset, uint size)
{
	stats::add_reversal(1);
	uint begin = offset;
	uint end = offset + size - 1;
	while (begin < end)
	{
		swap(array[begin], array[end]);
		++begin;
		--end;
	}
}
