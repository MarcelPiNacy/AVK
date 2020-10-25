#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vector>
#include "..\main_array.h"
#include <cassert>
#include "../enforce.h"
#include "vulkan_state.h"
#include "../defer.h"

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

void fill_infos()
{
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.queueFamilyIndexCount = 1;
	buffer_info.pQueueFamilyIndices = &graphics_queue_index;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	memory_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &array_command_buffer;
}

void main_array::initialize(uint size) noexcept
{
	if (buffer != VK_NULL_HANDLE)
		finalize();
	if (buffer_info.sType == 0)
		fill_infos();
	array_size = size;
	buffer_info.size = sizeof(element) * array_size;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	enforce(vkCreateBuffer(device, &buffer_info, nullptr, &buffer) == VK_SUCCESS);
	memory_info.allocationSize = buffer_info.size;
	memory_info.memoryTypeIndex = find_buffer_memory_type(buffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	enforce(vkAllocateMemory(device, &memory_info, nullptr, &memory) == VK_SUCCESS);
	enforce(vkMapMemory(device, memory, 0, buffer_info.size, 0, (void**)&mapping) == VK_SUCCESS);
}

void main_array::finalize() noexcept
{
	vkFreeMemory(device, memory, nullptr);
	vkDestroyBuffer(device, buffer, nullptr);
}

element& main_array::operator[](uint index) noexcept
{
	enforce(mapping != nullptr);
	enforce(index < size());
	return mapping[index];
}

uint main_array::size() noexcept
{
	return array_size;
}

element* main_array::begin() noexcept
{
	return mapping;
}

element* main_array::end() noexcept
{
	return mapping + array_size;
}

const element* main_array::cbegin() const noexcept
{
	return mapping;
}

const element* main_array::cend() const noexcept
{
	return mapping + array_size;
}

const element* main_array::begin() const noexcept
{
	return mapping;
}

const element* main_array::end() const noexcept
{
	return mapping + array_size;
}

element::operator uint() const noexcept
{
	stats::add_read();
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value;
}

element& element::operator=(const element& other) noexcept
{
	stats::add_read();
	stats::add_write();
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	internal_value = other.internal_value;
	return *this;
}

bool element::operator==(const element& other) const noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value == other.internal_value;
}

bool element::operator!=(const element& other) const noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value != other.internal_value;
}

bool element::operator<(const element& other) const noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value < other.internal_value;
}

bool element::operator>(const element& other) const noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value > other.internal_value;
}

bool element::operator<=(const element& other) const noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value <= other.internal_value;
}

bool element::operator>=(const element& other) const noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return internal_value >= other.internal_value;
}

sint compare(const element& left, const element& right) noexcept
{
	stats::add_comparisson();
	stats::add_read(2);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	return (sint)left.internal_value - (sint)right.internal_value;
}

void swap(element& left, element& right) noexcept
{
	stats::add_swap();
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	const element tmp = left;
	left = right;
	right = tmp;
}

void reverse(main_array& array, uint offset, uint size)
{
	stats::add_reversal(1);
	array_lock.lock_read();
	DEFER{ array_lock.unlock_read(); };
	uint begin = offset;
	uint end = offset + size - 1;
	while (begin < end)
	{
		swap(array[begin], array[end]);
		++begin;
		--end;
	}
}
