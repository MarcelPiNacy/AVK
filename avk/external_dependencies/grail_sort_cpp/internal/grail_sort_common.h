#pragma once
#include "util.h"
#include "small_sorts.h"



namespace grail_sort::detail
{
	template <typename Iterator, typename Int>
	constexpr Int gather_keys(
		Iterator begin,
		Int size,
		Int desired_key_count) GRAILSORT_NOTHROW
	{
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
	constexpr void merge_left_inplace(
		Iterator begin,
		Int left_size,
		Int right_size) GRAILSORT_NOTHROW
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
	constexpr void merge_right_inplace(
		Iterator begin,
		Int left_size,
		Int right_size) GRAILSORT_NOTHROW
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
	constexpr void merge_inplace(
		Iterator begin,
		Int left_size,
		Int right_size) GRAILSORT_NOTHROW
	{
		if (left_size < right_size)
			merge_left_inplace(begin, left_size, right_size);
		else
			merge_right_inplace(begin, left_size, right_size);
	}

	template <typename Iterator, typename Int>
	constexpr void merge_forward(
		Iterator begin,
		Int left_size,
		Int right_size,
		Int internal_buffer_offset) GRAILSORT_NOTHROW
	{
		Int left_offset = 0;
		Int right_offset = left_size;
		right_size += left_size;

		while (right_offset < right_size)
		{
			if (left_offset == left_size || *(begin + left_offset) > * (begin + right_offset))
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
	constexpr void merge_backward(
		Iterator begin,
		Int left_size,
		Int right_size,
		Int internal_buffer_offset) GRAILSORT_NOTHROW
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
	constexpr void smart_merge(
		Iterator begin,
		Int& ref_left_size,
		Int& ref_type,
		Int right_size,
		Int key_count) GRAILSORT_NOTHROW
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
	constexpr void smart_merge_inplace(
		Iterator begin,
		Int& ref_left_size,
		Int& ref_type,
		Int right_size) GRAILSORT_NOTHROW
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
	constexpr void merge_forward_using_external_buffer(
		Iterator begin,
		Int left_size,
		Int right_size,
		Int m) GRAILSORT_NOTHROW
	{
		Int left_offset = 0;
		Int right_offset = left_size;
		right_size += left_size;

		while (right_offset < right_size)
		{
			if (left_offset == left_size || *(begin + left_offset) > *(begin + right_offset))
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
	constexpr void smart_merge_using_external_buffer(
		Iterator begin,
		Int& ref_left_size,
		Int& ref_type,
		Int right_size,
		Int key_count) GRAILSORT_NOTHROW
	{
		Int buffer_offset = -key_count;
		Int left_offset = 0;
		Int right_offset = ref_left_size;
		Int middle_offset = right_offset;
		Int end_offset = right_offset + right_size;
		Int type = 1 - ref_type;

		while (left_offset < middle_offset && right_offset < end_offset)
		{
			if (compare(*(begin + buffer_offset), *(begin + left_offset)) - type < 0)
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
	constexpr void merge_buffers_forward_using_external_buffer(
		Iterator begin,
		Iterator keys,
		Iterator median,
		Int block_count,
		Int block_size,
		Int block_count_2,
		Int last) GRAILSORT_NOTHROW
	{
		if (block_count == 0)
			return merge_forward_using_external_buffer(begin, block_count_2 * block_size, last, -block_size);

		Int lrest = block_size;
		Int type = (Int)!(*keys < *median);
		Int current_block_offset = block_size;

		for (Int current_block = 1; current_block < block_count; ++current_block)
		{
			Int prest = current_block_offset - lrest;
			Int fnext = (Int)!(*(keys + current_block) < *median);
			if (fnext == type)
			{
				block_move(begin + (prest - block_size), begin + prest, lrest);
				prest = current_block_offset;
				lrest = block_size;
			}
			else
			{
				smart_merge_using_external_buffer(begin + prest, lrest, type, block_size, block_size);
			}

			current_block_offset += block_size;
		}

		Int prest = current_block_offset;
		if (last != 0)
		{
			Int plast = current_block_offset + block_size * block_count_2;
			if (type != 0)
			{
				block_move(begin + (prest - block_size), begin + prest, lrest);
				prest = current_block_offset;
				lrest = block_size * block_count_2;
				type = 0;
			}
			else
			{
				lrest += block_size * block_count_2;
			}

			merge_forward_using_external_buffer(begin + prest, lrest, last, -block_size);
		}
		else
		{
			block_move(begin + (prest - block_size), begin + prest, lrest);
		}
	}

	template <typename Iterator, typename Int>
	constexpr void merge_buffers_forward(
		Iterator begin,
		Iterator keys,
		Iterator median,
		Int block_count,
		Int block_size,
		bool has_buffer,
		Int block_count_2,
		Int last) GRAILSORT_NOTHROW
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

		Int lrest = block_size;
		Int frest = (Int)!(*keys < *median);
		Int prest = 0;
		Int p = block_size;

		for (Int current_block = 1; current_block < block_count; ++current_block)
		{
			Int prest = p - lrest;
			Int fnext = (Int)!(*(keys + current_block) < *median);
			if (fnext == frest)
			{
				if (has_buffer)
				{
					block_swap(begin + (prest - block_size), begin + prest, lrest);
				}
				prest = p;
				lrest = block_size;
			}
			else
			{
				if (has_buffer)
				{
					smart_merge(begin + prest, lrest, frest, block_size, block_size);
				}
				else
				{
					smart_merge_inplace(begin + prest, lrest, frest, block_size);
				}
			}
			p += block_size;
		}

		prest = p - lrest;
		if (last != 0)
		{
			const Int span = block_size * block_count_2;
			Int plast = p + span;
			if (frest != 0)
			{
				if (has_buffer)
				{
					block_swap(begin + (prest - block_size), begin + prest, lrest);
				}

				prest = p;
				lrest = span;
				frest = 0;
			}
			else
			{
				lrest += span;
			}

			if (has_buffer)
			{
				merge_forward(begin + prest, lrest, last, -block_size);
			}
			else
			{
				merge_inplace(begin + prest, lrest, last);
			}
		}
		else
		{
			if (has_buffer)
			{
				block_swap(begin + prest, begin + (prest - block_size), lrest);
			}
		}
	}

	template <typename Iterator, typename Int, bool DisableExternalBuffer>
	constexpr void build_blocks(
		Iterator begin,
		Int size,
		Int internal_buffer_size,
		Iterator external_buffer,
		Int external_buffer_size) GRAILSORT_NOTHROW
	{
		Int buffer_size = external_buffer_size;
		if (internal_buffer_size < external_buffer_size)
			buffer_size = internal_buffer_size;

		while (true)
		{
			const Int mask = buffer_size - 1;
			const Int next = buffer_size & mask;
			if (next == 0)
				break;
			buffer_size = next;
		}

		Int i = 2;
		if (buffer_size != 0)
		{
			block_move(external_buffer, begin - buffer_size, buffer_size);

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

			while (i < buffer_size)
			{
				const Int next = i * 2;
				Int offset = 0;

				for (; offset <= size - next; offset += next)
					merge_forward_using_external_buffer(begin + offset, i, i, -i);

				Int rest = size - offset;
				if (rest > i)
				{
					merge_forward_using_external_buffer(begin + offset, i, rest - i, -i);
				}
				else
				{
					while (offset < size)
					{
						move_construct(*(begin + (offset - i)), *(begin + offset));
						++offset;
					}
				}
				begin -= i;
				i = next;
			}
			block_move(begin + size, external_buffer, buffer_size);
		}
		else
		{
			for (Int j = 1; j < size; j += 2)
			{
				Int u = (Int)(*(begin + (j - 1)) > * (begin + j));
				swap(*(begin + (j - 3)), *(begin + (j - 1 + u)));
				swap(*(begin + (j - 2)), *(begin + (j - u)));
			}

			if ((size & 1) != 0)
			{
				swap(*(begin + (size - 1)), *(begin + (size - 3)));
			}

			begin -= 2;
		}

		while (i < internal_buffer_size)
		{
			const Int next = i * 2;
			Int p0 = 0;

			while (p0 <= size - next)
			{
				merge_forward(begin + p0, i, i, -i);
				p0 += next;
			}

			const Int rest = size - p0;
			if (rest > i)
			{
				merge_forward(begin + p0, i, rest - i, -i);
			}
			else
			{
				rotate(begin + (p0 - i), i, rest);
			}
			begin -= i;
			i = next;
		}

		const Int k2 = internal_buffer_size * 2;

		Int rest = size % k2;
		Int p = size - rest;
		if (rest <= internal_buffer_size)
		{
			rotate(begin + p, rest, internal_buffer_size);
		}
		else
		{
			merge_backward(begin + p, internal_buffer_size, rest - internal_buffer_size, internal_buffer_size);
		}


		while (p != 0)
		{
			p -= k2;
			merge_backward(begin + p, internal_buffer_size, internal_buffer_size, internal_buffer_size);
		}
	}

	template <typename Iterator, typename Int>
	constexpr void combine_blocks(
		Iterator begin,
		Iterator keys,
		Int size,
		Int range_size,
		Int block_size,
		bool has_buffer,
		Iterator external_buffer) GRAILSORT_NOTHROW
	{
		const auto nil_iterator = Iterator();

		const Int merged_size = range_size * 2;
		Int block_count = size / merged_size;
		Int rest = size % merged_size;
		Int key_count = merged_size / block_size;

		if (rest <= range_size)
		{
			size -= rest;
			rest = 0;
		}

		if (external_buffer != nil_iterator)
		{
			block_move(external_buffer, begin - block_size, block_size);
		}

		for (Int i = 0; i <= block_count; ++i)
		{
			const bool flag = i == block_count;
			if (flag && rest == 0)
				break;

			const Int count = (flag ? rest : merged_size) / block_size;

			insertion_sort_unstable(keys, count + (Int)flag);

			Int median = range_size / block_size;
			const Iterator local_begin = begin + i * merged_size;

			for (Int j = 1; j < count; ++j)
			{
				const Int target_0 = j - 1;
				Int target = target_0;
				for (Int v = j; v < count; ++v)
				{
					const auto cmp = compare(*(local_begin + target * block_size), *(local_begin + v * block_size));
					if (cmp > 0 || (cmp == 0 && *(keys + target) > * (keys + v)))
						target = v;
				}

				if (target != target_0)
				{
					block_swap(local_begin + target_0 * block_size, local_begin + target * block_size, block_size);
					swap(*(keys + target_0), *(keys + target));
					if (median == target_0 || median == target)
						median ^= target_0 ^ target;
				}
			}

			Int bk2 = 0;
			const Int last = flag ? rest % block_size : 0;

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

			block_move(begin - block_size, external_buffer, block_size);
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

	template <typename Iterator, typename Int>
	constexpr void lazy_merge_sort(
		Iterator begin,
		Int size) GRAILSORT_NOTHROW
	{
		for (Int i = 1; i < size; i += 2)
		{
			if (*(begin + (i - 1)) > * (begin + i))
			{
				swap(*(begin + (i - 1)), *(begin + i));
			}
		}

		for (Int block_size = 2; block_size < size;)
		{
			const Int step_size = block_size * 2;
			Int left_offset = 0;
			for (Int p1 = size - step_size; left_offset <= p1; left_offset += step_size)
				merge_inplace(begin + left_offset, block_size, block_size);
			const Int rest = size - left_offset;
			if (rest > block_size)
				merge_inplace(begin + left_offset, block_size, rest - block_size);
			block_size = step_size;
		}
	}

	template <typename Iterator, typename Int, bool DisableExternalBuffer>
	constexpr void entry_point(
		Iterator begin,
		Int size,
		Iterator external_buffer,
		Int external_buffer_size) GRAILSORT_NOTHROW
	{
		if (size < 16)
			return insertion_sort_stable(begin, size);

		const auto nil_iterator = Iterator();

		Int block_size = 4;
		while (block_size * block_size < size)
			block_size *= 2;

		Int key_count = 1 + (size - 1) / block_size;
		const Int desired_key_count = key_count + block_size;
		Int found_key_count = gather_keys(begin, size, desired_key_count);
		const bool has_buffer = found_key_count >= desired_key_count;

		if (!has_buffer)
		{
			if (key_count < 4)
				return lazy_merge_sort(begin, size);

			key_count = block_size;
			while (key_count > found_key_count)
				key_count /= 2;
			block_size = 0;
		}

		const Int offset = block_size + key_count;
		const Iterator values = begin + offset;
		const Int range = size - offset;
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
				break;

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
						local_block_size /= 2;
				}
			}

			const Iterator buffer_iterator =
				local_has_buffer && (local_block_size <= external_buffer_size) ?
				nil_iterator :
				external_buffer;

			combine_blocks(values, begin, range, internal_buffer_size, local_block_size, local_has_buffer, buffer_iterator);
		}

		insertion_sort_unstable(begin, offset); //All items are unique, stability isn't required.
		merge_inplace(begin, offset, range);
	}
}