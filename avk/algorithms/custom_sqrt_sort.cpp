#include "all.h"

using I = item*;
using T = item;
using mask_type = uint32_t;

static constexpr size_t tiny_buffer_size = 4096 / sizeof(T);
static thread_local T tiny_buffer[tiny_buffer_size];

bool cmp_eq(T& lhs, T& rhs) { return lhs == rhs; }
bool cmp_ne(T& lhs, T& rhs) { return lhs != rhs; }
bool cmp_lt(T& lhs, T& rhs) { return lhs < rhs; }
bool cmp_gt(T& lhs, T& rhs) { return lhs > rhs; }
bool cmp_le(T& lhs, T& rhs) { return lhs <= rhs; }
bool cmp_ge(T& lhs, T& rhs) { return lhs >= rhs; }

size_t fast_sqrt(size_t count)
{
	uint8_t shift = 64 - (uint8_t)__lzcnt64(count);
	if (shift & 1)
		++shift;
	size_t r = 0;
	do
	{
		shift -= 2;
		r *= 2;
		r |= 1;
		r ^= r * r > (count >> shift);
	} while (shift != 0);
	return r;
}

void build_small_runs(I begin, I end, size_t min_run_size)
{
	while (true)
	{
	}
}

void custom_sqrt_sort(main_array array)
{
	size_t bit_count = fast_sqrt(array.size());
	size_t mask_type_size_mask = sizeof(mask_type) * 8 - 1;
	size_t mask_count = ((bit_count + mask_type_size_mask) & ~mask_type_size_mask) / sizeof(mask_type);
	size_t required_buffer_size = mask_count * 8;
	required_buffer_size += bit_count * sizeof(T);
	void* buffer = malloc(required_buffer_size);
	mask_type* control_buffer = (mask_type*)buffer;
	T* spare_buffer = (T*)(control_buffer + mask_count);
	(void)std::move(array.begin(), array.begin() + bit_count, spare_buffer);
	
}