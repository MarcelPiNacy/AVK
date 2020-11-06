/*
	MIT License

	Copyright (c) 2013 Andrey Astrelin
	Copyright (c) 2020 Marcel Pi Nacy

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#pragma once
#include "util.h"
#include "small_sorts.h"



namespace grail_sort::detail
{
	template <typename Iterator, typename Int>
	constexpr Int gather_keys(Iterator begin, Iterator end, Int desired_key_count) GRAILSORT_NOTHROW
	{
		const auto size = std::distance(begin, end);
		Int first_key = 0;
		Int found_count = 1;
		for (Int i = 1; i < size && found_count < desired_key_count; ++i)
		{
			const Int target = lower_bound(begin + first_key, found_count, begin + i);
			if (target == found_count || *(begin + i) != *(begin + (first_key + target)))
			{
				rotate(begin + first_key, found_count, i - (first_key + found_count));
				first_key = i - found_count;
				rotate_single<Iterator, Int>(begin + (first_key + target), found_count - target);
				++found_count;
			}
		}

		rotate(begin, first_key, found_count);
		return found_count;
	}

	template <typename Iterator, typename Int>
	constexpr void merge_left_inplace(Iterator begin, Int left_size, Int right_size) GRAILSORT_NOTHROW
	{
		while (left_size != 0)
		{
			const Int target = lower_bound(begin + left_size, right_size, begin);
			if (target != 0)
			{
				rotate(begin, left_size, target);
				begin += target;
				right_size -= target;
			}

			if (right_size == 0)
			{
				break;
			}

			do
			{
				++begin;
				--left_size;
			} while (left_size != 0 && *begin <= *(begin + left_size));
		}
	}

	template <typename Iterator, typename Int>
	constexpr void merge_right_inplace(Iterator begin, Int left_size, Int right_size) GRAILSORT_NOTHROW
	{
		while (right_size != 0)
		{
			const Int target = upper_bound(begin, left_size, begin + (left_size + right_size - 1));
			if (target != left_size)
			{
				rotate(begin + target, left_size - target, right_size);
				left_size = target;
			}

			if (left_size == 0)
			{
				break;
			}

			do
			{
				--right_size;
			} while (right_size != 0 && *(begin + (left_size - 1)) <= *(begin + (left_size + right_size - 1)));
		}
	}

	template <typename Iterator, typename Int>
	constexpr void merge_inplace(Iterator begin, Int left_size, Int right_size) GRAILSORT_NOTHROW
	{
		if (left_size < right_size)
			merge_left_inplace(begin, left_size, right_size);
		else
			merge_right_inplace(begin, left_size, right_size);
	}

	template <typename Iterator, typename Int>
	constexpr void merge_forward(Iterator begin, Int left_size, Int right_size, Int internal_buffer_offset) GRAILSORT_NOTHROW
	{
		Int left_offset = 0;
		Int right_offset = left_size;
		right_size += left_size;

		while (right_offset < right_size)
		{
			if (left_offset == left_size || *(begin + left_offset) > *(begin + right_offset))
			{
				swap(*(begin + internal_buffer_offset), *(begin + right_offset));
				++right_offset;
			}
			else
			{
				swap(*(begin + internal_buffer_offset), *(begin + left_offset));
				++left_offset;
			}
			++internal_buffer_offset;
		}

		if (internal_buffer_offset != left_offset)
		{
			block_swap(begin + internal_buffer_offset, begin + left_offset, left_size - left_offset);
		}
	}

	template <typename Iterator, typename Int>
	constexpr void merge_backward(Iterator begin, Int left_size, Int right_size, Int internal_buffer_offset) GRAILSORT_NOTHROW
	{
		Int left_offset = left_size - 1;
		Int right_offset = right_size + left_offset;
		Int buffer_offset = right_offset + internal_buffer_offset;

		while (left_offset >= 0)
		{
			if (right_offset < left_size || *(begin + left_offset) > * (begin + right_offset))
			{
				swap(*(begin + buffer_offset), *(begin + left_offset));
				--left_offset;
			}
			else
			{
				swap(*(begin + buffer_offset), *(begin + right_offset));
				--right_offset;
			}
			--buffer_offset;
		}

		if (right_offset != buffer_offset)
		{
			while (right_offset >= left_size)
			{
				swap(*(begin + buffer_offset), *(begin + right_offset));
				--buffer_offset;
				--right_offset;
			}
		}
	}

	template <typename Iterator, typename Int>
	constexpr void smart_merge(Iterator begin, Int& ref_left_size, Int& ref_type, Int right_size, Int key_count) GRAILSORT_NOTHROW
	{
		Int buffer_offset = -key_count;
		Int left_offset = 0;
		Int right_offset = ref_left_size;
		Int middle_offset = right_offset;
		Int end_offset = right_offset + right_size;
		Int type = 1 - ref_type;

		while (left_offset < middle_offset && right_offset < end_offset)
		{
			if (compare(*(begin + left_offset), *(begin + right_offset)) - type < 0)
			{
				swap(*(begin + buffer_offset), *(begin + left_offset));
				++left_offset;
			}
			else
			{
				swap(*(begin + buffer_offset), *(begin + right_offset));
				++right_offset;
			}
			++buffer_offset;
		}

		if (left_offset < middle_offset)
		{
			ref_left_size = middle_offset - left_offset;
			while (left_offset < middle_offset)
			{
				--middle_offset;
				--end_offset;
				swap(*(begin + middle_offset), *(begin + end_offset));
			}
		}
		else
		{
			ref_left_size = end_offset - right_offset;
			ref_type = type;
		}
	}

	template <typename Iterator, typename Int>
	constexpr void smart_merge_inplace(Iterator begin, Int& ref_left_size, Int& ref_type, Int right_size) GRAILSORT_NOTHROW
	{
		if (right_size == 0)
			return;

		Int left_size = ref_left_size;
		Int type = 1 - ref_type;
		if (left_size != 0 && (compare(*(begin + (left_size - 1)), *(begin + left_size)) - type) >= 0)
		{
			while (left_size != 0)
			{
				const Int target = type ?
					lower_bound(begin + left_size, right_size, begin) :
					upper_bound(begin + left_size, right_size, begin);

				if (target != 0)
				{
					rotate(begin, left_size, target);
					begin += target;
					right_size -= target;
				}

				if (right_size == 0)
				{
					ref_left_size = left_size;
					return;
				}

				do
				{
					++begin;
					--left_size;
				} while (left_size != 0 && compare(*begin, *(begin + left_size)) - type < 0);
			}
		}
		ref_left_size = right_size;
		ref_type = type;
	}

	template <typename Iterator, typename Int>
	constexpr void merge_forward_using_external_buffer(Iterator begin, Int left_size, Int right_size, Int m) GRAILSORT_NOTHROW
	{
		Int left_offset = 0;
		Int right_offset = left_size;
		right_size += left_size;

		while (right_offset < right_size)
		{
			if (left_offset == left_size || *(begin + left_offset) > * (begin + right_offset))
			{
				move_construct(*(begin + m), *(begin + right_offset));
				++right_offset;
			}
			else
			{
				move_construct(*(begin + m), *(begin + left_offset));
				++left_offset;
			}
			++m;
		}

		if (m != left_offset)
		{
			while (left_offset < left_size)
			{
				move_construct(*(begin + m), *(begin + left_offset));
				++m;
				++left_offset;
			}
		}
	}

	template <typename Iterator, typename Int>
	constexpr void smart_merge_using_external_buffer(Iterator begin, Int& ref_left_size, Int& ref_type, Int right_size, Int key_count) GRAILSORT_NOTHROW
	{
		Int buffer_offset = -key_count;
		Int left_offset = 0;
		Int right_offset = ref_left_size;
		Int middle_offset = right_offset;
		Int end_offset = right_offset + right_size;
		Int type = 1 - ref_type;

		while (left_offset < middle_offset && right_offset < end_offset)
		{
			if (compare(*(begin + left_offset), *(begin + right_offset)) - type < 0)
			{
				move_construct(*(begin + buffer_offset), *(begin + left_offset));
				++left_offset;
			}
			else
			{
				move_construct(*(begin + buffer_offset), *(begin + right_offset));
				++right_offset;
			}
			++buffer_offset;
		}

		if (left_offset < middle_offset)
		{
			ref_left_size = middle_offset - left_offset;
			while (left_offset < middle_offset)
			{
				--end_offset;
				--middle_offset;
				move_construct(*(begin + end_offset), *(begin + middle_offset));
			}
		}
		else
		{
			ref_left_size = end_offset - right_offset;
			ref_type = type;
		}
	}

	template <typename Iterator, typename Int>
	constexpr void merge_buffers_forward_using_external_buffer(Iterator begin, Iterator keys, Iterator median, Int block_count, Int block_size, Int block_count_2, Int last) GRAILSORT_NOTHROW
	{
		if (block_count == 0)
		{
			merge_forward_using_external_buffer(begin, block_count_2 * block_size, last, -block_size);
			return;
		}

		auto left_rest = block_size;
		auto type = (Int)!(*keys < *median);
		auto current_block_offset = block_size;

		for (Int current_block = 1; current_block < block_count; ++current_block)
		{
			auto prest = current_block_offset - left_rest;
			auto fnext = (Int)!(*(keys + current_block) < *median);

			if (fnext == type)
			{
				block_move(begin + prest, begin + (prest + left_rest), begin + (prest - block_size));
				prest = current_block_offset;
				left_rest = block_size;
			}
			else
			{
				smart_merge_using_external_buffer(begin + prest, left_rest, type, block_size, block_size);
			}

			current_block_offset += block_size;
		}

		auto prest = current_block_offset - left_rest;

		if (last != 0)
		{
			const auto k = block_size * block_count_2;
			if (type != 0)
			{
				block_move(begin + prest, begin + prest + left_rest, begin + (prest - block_size));
				prest = current_block_offset;
				left_rest = k;
			}
			else
			{
				left_rest += k;
			}

			merge_forward_using_external_buffer(begin + prest, left_rest, last, -block_size);
		}
		else
		{
			block_move(begin + prest, begin + prest + left_rest, begin + (prest - block_size));
		}
	}

	template <typename Iterator, typename Int>
	constexpr void merge_buffers_forward(Iterator begin, Iterator keys, Iterator median, Int block_count, Int block_size, bool has_buffer, Int block_count_2, Int last) GRAILSORT_NOTHROW
	{
		if (block_count == 0)
		{
			const Int left_size = block_count_2 * block_size;

			if (has_buffer)
			{
				merge_forward(begin, left_size, last, -block_size);
			}
			else
			{
				merge_inplace(begin, left_size, last);
			}

			return;
		}

		Int current_block_size = block_size;
		Int current_block_origin = (Int)!(*keys < *median);
		Int current_block_end = block_size;
		Int current_block_offset;

		for (Int current_block_index = 1; current_block_index < block_count; ++current_block_index)
		{
			current_block_offset = current_block_end - current_block_size;
			Int next_block = (Int)!(*(keys + current_block_index) < *median);

			if (next_block == current_block_origin)
			{
				if (has_buffer)
				{
					block_swap(begin + (current_block_offset - block_size), begin + current_block_offset, current_block_size);
				}

				current_block_offset = current_block_end;
				current_block_size = block_size;
			}
			else
			{
				if (has_buffer)
				{
					smart_merge(begin + current_block_offset, current_block_size, current_block_origin, block_size, block_size);
				}
				else
				{
					smart_merge_inplace(begin + current_block_offset, current_block_size, current_block_origin, block_size);
				}
			}

			current_block_end += block_size;
		}

		current_block_offset = current_block_end - current_block_size;

		if (last != 0)
		{
			const Int k = block_size * block_count_2;
			Int last = current_block_end + k;

			if (current_block_origin != 0)
			{
				if (has_buffer)
				{
					block_swap(begin + (current_block_offset - block_size), begin + current_block_offset, current_block_size);
				}

				current_block_offset = current_block_end;
				current_block_size = k;
			}
			else
			{
				current_block_size += k;
			}

			if (has_buffer)
			{
				merge_forward(begin + current_block_offset, current_block_size, last, -block_size);
			}
			else
			{
				merge_inplace(begin + current_block_offset, current_block_size, last);
			}
		}
		else
		{
			if (has_buffer)
			{
				block_swap(begin + current_block_offset, begin + (current_block_offset - block_size), current_block_size);
			}
		}
	}

	template <typename Iterator, typename Int, bool DisableExternalBuffer>
	constexpr void build_blocks(Iterator begin, Int size, Int block_size, Iterator external_buffer, Int external_buffer_size) GRAILSORT_NOTHROW
	{
		Int buffer_size = external_buffer_size;
		if (block_size < external_buffer_size)
			buffer_size = block_size;

		while (true)
		{
			const Int mask = buffer_size - 1;
			const Int next = buffer_size & mask;

			if (next == 0)
				break;

			buffer_size = next;
		}

		Int current_block_size = 2;

		if (buffer_size != 0)
		{
			block_move(begin - buffer_size, begin, external_buffer);

			for (Int j = 1; j < size; j += 2)
			{
				const bool flag = *(begin + (j - 1)) > * (begin + j);
				move_construct(*(begin + (j - 3)), *(begin + (j - 1 + (Int)flag)));
				move_construct(*(begin + (j - 2)), *(begin + (j - (Int)flag)));
			}

			if ((size & 1) != 0)
			{
				move_construct(*(begin + (size - 3)), *(begin + (size - 1)));
			}

			begin -= 2;

			while (current_block_size < buffer_size)
			{
				const Int next = current_block_size * 2;
				Int offset = 0;

				for (; offset <= size - next; offset += next)
				{
					merge_forward_using_external_buffer(begin + offset, current_block_size, current_block_size, -current_block_size);
				}

				const Int rest = size - offset;

				if (rest > current_block_size)
				{
					merge_forward_using_external_buffer(begin + offset, current_block_size, rest - current_block_size, -current_block_size);
				}
				else
				{
					while (offset < size)
					{
						move_construct(*(begin + (offset - current_block_size)), *(begin + offset));
						++offset;
					}
				}

				begin -= current_block_size;
				current_block_size = next;
			}

			block_move(external_buffer, external_buffer + buffer_size, begin + size);
		}
		else
		{
			for (Int j = 1; j < size; j += 2)
			{
				const Int u = (Int)(*(begin + (j - 1)) > * (begin + j));
				swap(*(begin + (j - 3)), *(begin + (j - 1 + u)));
				swap(*(begin + (j - 2)), *(begin + (j - u)));
			}

			if ((size & 1) != 0)
			{
				swap(*(begin + (size - 1)), *(begin + (size - 3)));
			}

			begin -= 2;
		}

		while (current_block_size < block_size)
		{
			const Int next = current_block_size * 2;
			Int p0 = 0;

			while (p0 <= size - next)
			{
				merge_forward(begin + p0, current_block_size, current_block_size, -current_block_size);
				p0 += next;
			}

			const Int rest = size - p0;
			
			if (rest > current_block_size)
			{
				merge_forward(begin + p0, current_block_size, rest - current_block_size, -current_block_size);
			}
			else
			{
				rotate(begin + (p0 - current_block_size), current_block_size, rest);
			}

			begin -= current_block_size;
			current_block_size = next;
		}

		const Int k2 = block_size * 2;

		Int rest = smart_modulo(size, k2);
		Int p = size - rest;

		if (rest <= block_size)
		{
			rotate(begin + p, rest, block_size);
		}
		else
		{
			merge_backward(begin + p, block_size, rest - block_size, block_size);
		}

		while (p != 0)
		{
			p -= k2;
			merge_backward(begin + p, block_size, block_size, block_size);
		}
	}

	template <typename Iterator, typename Int>
	constexpr void combine_blocks(Iterator begin, Iterator keys, Int size, Int range_size, Int block_size, bool has_buffer, Iterator external_buffer) GRAILSORT_NOTHROW
	{
		const auto nil_iterator = Iterator();

		const Int merged_size = range_size * 2;
		Int block_count = size / merged_size;
		Int rest = smart_modulo(size, merged_size);
		Int key_count = merged_size / block_size;

		if (rest <= range_size)
		{
			size -= rest;
			rest = 0;
		}

		if (external_buffer != nil_iterator)
		{
			block_move(begin - block_size, begin, external_buffer);
		}

		for (Int current_block = 0; current_block <= block_count; ++current_block)
		{
			const bool flag = current_block == block_count;
			if (flag && rest == 0)
				break;

			const Int count = (flag ? rest : merged_size) / block_size;
			const Int last = flag ? smart_modulo(rest, block_size) : 0;

			insertion_sort_unstable(keys, keys + count + (Int)flag);

			Int median = range_size / block_size;
			const Iterator local_begin = begin + current_block * merged_size;

			for (Int j = 1; j < count; ++j)
			{
				const Int target_0 = j - 1;
				Int target = target_0;
				for (Int v = j; v < count; ++v)
				{
					const auto cmp = compare(*(local_begin + target * block_size), *(local_begin + v * block_size));
					
					if (cmp > 0 || (cmp == 0 && *(keys + target) > * (keys + v)))
					{
						target = v;
					}
				}

				if (target != target_0)
				{
					block_swap(local_begin + target_0 * block_size, local_begin + target * block_size, block_size);
					swap(*(keys + target_0), *(keys + target));
					
					if (median == target_0 || median == target)
					{
						median ^= target_0 ^ target;
					}
				}
			}

			Int bk2 = 0;

			if (last != 0)
			{
				while (bk2 < count && *(local_begin + count * block_size) < *(local_begin + (count - bk2 - 1) * block_size))
				{
					++bk2;
				}
			}

			if (external_buffer != nil_iterator)
			{
				merge_buffers_forward_using_external_buffer(local_begin, keys, keys + median, count - bk2, block_size, bk2, last);
			}
			else
			{
				merge_buffers_forward(local_begin, keys, keys + median, count - bk2, block_size, has_buffer, bk2, last);
			}
		}

		--size;

		if (external_buffer != nil_iterator)
		{
			while (size >= 0)
			{
				move_construct(*(begin + size), *(begin + size - block_size));
				--size;
			}

			block_move(external_buffer, external_buffer + block_size, begin - block_size);
		}
		else if (has_buffer)
		{
			while (size >= 0)
			{
				swap(*(begin + size), *(begin + size - block_size));
				--size;
			}
		}
	}

	template <typename Iterator>
	constexpr void lazy_merge_sort(Iterator begin, Iterator end) GRAILSORT_NOTHROW
	{
		for (Iterator e = begin + 1; e < end; e += 2)
		{
			Iterator prev = e - 1;
			if (*prev > * e)
			{
				swap(*prev, *e);
			}
		}

		const auto size = std::distance(begin, end);
		using Int = std::remove_const_t<decltype(size)>;

		for (Int block_size = 2; block_size < size;)
		{
			const Int step_size = block_size * 2;
			Int left_offset = 0;
			
			for (Int p1 = size - step_size; left_offset <= p1; left_offset += step_size)
			{
				merge_inplace(begin + left_offset, block_size, block_size);
			}
			
			const Int rest = size - left_offset;

			if (rest > block_size)
			{
				merge_inplace(begin + left_offset, block_size, rest - block_size);
			}

			block_size = step_size;
		}
	}

	template <typename Iterator, typename Int, bool DisableExternalBuffer>
	constexpr void entry_point(Iterator begin, Int size, Iterator external_buffer, Int external_buffer_size) GRAILSORT_NOTHROW
	{
		if (size < 64)
		{
			insertion_sort_stable(begin, begin + size);
			return;
		}

		const auto nil_iterator = Iterator();

		Int block_size = 8;

		while (block_size * block_size < size)
		{
			block_size *= 2;
		}

		Int key_count = 1 + (size - 1) / block_size;
		const Int desired_key_count = key_count + block_size;
		Int found_key_count = gather_keys(begin, begin + size, desired_key_count);
		const bool has_buffer = found_key_count >= desired_key_count;

		if (!has_buffer)
		{
			if (key_count < 4)
			{
				lazy_merge_sort<Iterator>(begin, begin + size);
				return;
			}

			key_count = block_size;
			
			while (key_count > found_key_count)
			{
				key_count /= 2;
			}

			block_size = 0;
		}

		const Int key_buffer_size = block_size + key_count;
		const Iterator values = begin + key_buffer_size;
		const Int range = size - key_buffer_size;
		Int internal_buffer_size = key_count;

		if (has_buffer)
		{
			internal_buffer_size = block_size;
			build_blocks<Iterator, Int, true>(values, range, internal_buffer_size, external_buffer, external_buffer_size);
		}
		else
		{
			build_blocks<Iterator, Int, false>(values, range, internal_buffer_size, nil_iterator, 0);
		}

		while (true)
		{
			internal_buffer_size *= 2;
			
			if (internal_buffer_size >= range)
			{
				break;
			}

			Int local_block_size = block_size;
			bool local_has_buffer = has_buffer;

			if (!local_has_buffer)
			{
				if (key_count > 4 && key_count / 8 * key_count >= internal_buffer_size)
				{
					local_block_size = key_count / 2;
					local_has_buffer = true;
				}
				else
				{
					Int local_range_size = 1;

					for (intmax_t s = (intmax_t)internal_buffer_size * key_count / 2; local_range_size < key_count && s != 0; s /= 8)
					{
						local_range_size *= 2;
					}

					local_block_size = (2 * internal_buffer_size) / local_range_size;
				}
			}
			else
			{
				if (external_buffer_size != 0)
				{
					while (local_block_size > external_buffer_size && local_block_size * local_block_size > 2 * internal_buffer_size)
					{
						local_block_size /= 2;
					}
				}
			}

			Iterator buffer = nil_iterator;
			
			if (local_has_buffer && local_block_size <= external_buffer_size)
			{
				buffer = external_buffer;
			}

			combine_blocks(values, begin, range, internal_buffer_size, local_block_size, local_has_buffer, buffer);
		}

		insertion_sort_unstable(begin, begin + key_buffer_size);
		merge_inplace(begin, key_buffer_size, range);
	}
}