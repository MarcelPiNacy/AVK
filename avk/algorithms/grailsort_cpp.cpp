#include "all.h"

/*
	MIT License
	
	Copyright (c) 2013 Andrei Astrelin
	Copyright (c) 2021 Marcel Pi Nacy
	
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

/*
	This file contains a C++17 implementation of Andrei Astrelin's GrailSort, a block merge sorting algorithm, with some major optimizations.
*/

#include <new>
#include <limits>
#include <iterator>
#include <algorithm>
#include <type_traits>

#if !defined(GRAILSORT_DEBUG) && defined(_DEBUG) && !defined(NDEBUG)
#include <cassert>
#define GRAILSORT_DEBUG
#define GRAILSORT_INVARIANT(expression) assert(expression)
#else
#ifdef _MSVC_LANG
#define GRAILSORT_INVARIANT(expression) __assume((expression))
#elif defined(__clang__) || defined(__GNUC__)
#define GRAILSORT_INVARIANT(expression) __builtin_assume((expression))
#else
#define GRAILSORT_INVARIANT(expression)
#endif
#endif

namespace grailsort
{
	namespace detail
	{
		template <typename J>
		struct grail_sort_helper_base
		{
			J external_buffer_begin, external_buffer_end;
		};

		template <>
		struct grail_sort_helper_base<void> { };

		template <
			typename I, // RandomAccessIterator
			typename J> // BufferIterator
		struct helper_type : grail_sort_helper_base<J>
		{
			using base = grail_sort_helper_base<J>;
			using iterator_traits = std::iterator_traits<I>;
			using value_type = typename iterator_traits::value_type;
			using difference_type = typename iterator_traits::difference_type;
			using size_type = std::make_unsigned_t<difference_type>;

			helper_type() = default;
			helper_type(const helper_type&) = delete;
			helper_type& operator=(const helper_type&) = delete;
			~helper_type() = default;

			template <typename T>
			constexpr helper_type(T buffer_begin, T buffer_end)
				: base{ buffer_begin, buffer_end }
			{
			}

			static constexpr uint_fast8_t log2_of(size_t size)
			{
				uint_fast8_t r = 0;
				for (; (size & 1) == 0; size >>= 1)
					++r;
				return r;
			}

			static constexpr size_type sqrt_of(size_type size)
			{
				size_type r = 1;
				while (r * r < size)
					r *= 2;
				return r;
			}

			static constexpr bool is_pow2(size_type size)
			{
				uint_fast8_t k = 0;
				for (; size != 0; size >>= 1)
					k += static_cast<uint_fast8_t>(size & 1);
				return k == 1;
			}

			static constexpr size_type distance(I begin, I end)
			{
				GRAILSORT_INVARIANT(begin <= end);
				return static_cast<size_type>(std::distance(begin, end));
			}

			template <typename T>
			static constexpr void iter_move(I left, T&& right)
			{
				new (&*left) value_type(std::forward<T>(right));
			}

			static constexpr void iter_move(I left, I right)
			{
				iter_move(left, std::move(*right));
			}

			template <typename I2 = I>
			static constexpr void swap_range(I from_begin, I from_end, I2 to_begin)
			{
				GRAILSORT_INVARIANT(from_begin <= from_end);
				while (from_begin != from_end)
				{
					std::iter_swap(from_begin, to_begin);
					++from_begin;
					++to_begin;
				}
			}

			template <typename I2 = I>
			static constexpr void move_range(I from_begin, I from_end, I2 to_begin)
			{
				GRAILSORT_INVARIANT(from_begin <= from_end);
				while (from_begin != from_end)
				{
					iter_move(from_begin, to_begin);
					++from_begin;
					++to_begin;
				}
			}

			enum : bool
			{
				USING_EXTERNAL_BUFFER = !std::is_void<J>::value,
				CMOV_AVAILABLE = sizeof(I) <= sizeof(void*) && std::is_trivially_copyable<I>::value
			};

			enum : size_type
			{
				EXPECTED_PAGE_SIZE				= static_cast<size_type>(8192),
				CACHE_LINE_SIZE					= static_cast<size_type>(std::hardware_constructive_interference_size),
				SMALL_SORT_THRESHOLD			= static_cast<size_type>(std::max<size_t>(CACHE_LINE_SIZE / sizeof(value_type), 8)),
				SMALL_SORT_THRESHOLD_LOG2		= static_cast<size_type>(log2_of(SMALL_SORT_THRESHOLD)),
				MEDIUM_SORT_THRESHOLD			= static_cast<size_type>(SMALL_SORT_THRESHOLD * 4),
				LINEAR_SEARCH_THRESHOLD			= static_cast<size_type>(SMALL_SORT_THRESHOLD),
				BASIC_INSERTION_SORT_THRESHOLD	= static_cast<size_type>(6),
				GATHER_UNIQUE_SIMPLE_THRESHOLD	= static_cast<size_type>(CACHE_LINE_SIZE / sizeof(value_type))
			};

			constexpr void insert_backward(I target, I from)
			{
				GRAILSORT_INVARIANT(target <= from);
				value_type tmp = std::move(*from);
				--from;
				for (; from >= target; --from)
					iter_move(from + 1, from);
				iter_move(target, tmp);
			}

			constexpr void rotate(I begin, I new_begin, I end)
			{
				GRAILSORT_INVARIANT(begin <= end);
				GRAILSORT_INVARIANT(new_begin <= end);
				GRAILSORT_INVARIANT(begin <= new_begin);
				size_type left_size = distance(begin, new_begin);
				size_type right_size = distance(new_begin, end);
				while (left_size != 0 && right_size != 0)
				{
					I middle = std::next(begin, left_size);
					if (left_size <= right_size)
					{
						swap_range(begin, middle, middle);
						begin = middle;
						right_size -= left_size;
					}
					else
					{
						swap_range(std::next(begin, left_size - right_size), middle, middle);
						left_size -= right_size;
					}
				}
			}

			template <typename P>
			constexpr I linear_search_forward(I begin, I end, I key, P&& compare)
			{
				while (begin != end && compare(begin, key))
					++begin;
				return begin;
			}

			enum : uint_fast8_t
			{
				HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT = 1,
				HYBRID_SEARCH_OPTIONS_BACKWARD_BIT = 2
			};
			using hybrid_search_flags = uint_fast8_t;

			template <hybrid_search_flags Flags, typename P>
			constexpr I hybrid_search(I begin, I end, I key, P&& compare)
			{
				constexpr bool all_unique = (Flags & HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT) != 0;
				constexpr bool backward = (Flags & HYBRID_SEARCH_OPTIONS_BACKWARD_BIT) != 0;
				if (distance(begin, end) <= LINEAR_SEARCH_THRESHOLD)
					return linear_search_forward(begin, end, key, compare);
				I i = begin;
				begin += LINEAR_SEARCH_THRESHOLD;
				for (; i != begin; ++i)
					if (!compare(i, key))
						return i;
				if (begin == end)
					return begin;
				for (size_type increment = 1; i < end; increment += increment)
				{
					if constexpr (all_unique)
						if (*i == *key)
							return i;
					if (!compare(i, key))
						break;
					i = std::next(i, increment);
				}
				if (i > end)
					i = end;
				while (true)
				{
					size_type d = distance(begin, end);
					I i = std::next(begin, d / 2);
					if constexpr (all_unique)
						if (*i == *key)
							return i;
					if (d <= LINEAR_SEARCH_THRESHOLD)
						break;
					if (compare(i, key))
						begin = std::next(i, 1);
					else
						end = i;
				}
				return linear_search_forward(begin, end, key, compare);
			}

			template <hybrid_search_flags Flags = 0>
			constexpr I lower_bound(I begin, I end, I key)
			{
				return hybrid_search<Flags>(begin, end, key, [](I left, I right)
				{
					return *left < *right;
				});
			}

			template <hybrid_search_flags Flags = 0>
			constexpr I upper_bound(I begin, I end, I key)
			{
				return hybrid_search<Flags>(begin, end, key, [](I left, I right)
				{
					return !(*left > *right);
				});
			}

			constexpr size_type gather_unique_basic(I begin, I end, size_type expected_unique_count)
			{
				size_type unique_count = 1;
				I internal_buffer_end = begin + 1;
				for (I i = internal_buffer_end; i != end && unique_count != expected_unique_count; ++i)
				{
					I target = lower_bound<HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT | HYBRID_SEARCH_OPTIONS_BACKWARD_BIT>(begin, internal_buffer_end, i);
					if (target == internal_buffer_end || *i != *target)
					{
						insert_backward(target, i);
						++unique_count;
						++internal_buffer_end;
					}
				}
				return unique_count;
			}

			constexpr size_type gather_unique(I begin, I end, size_type expected_unique_count)
			{
				if (distance(begin, end) <= GATHER_UNIQUE_SIMPLE_THRESHOLD)
					return gather_unique_basic(begin, end, expected_unique_count);
				size_type unique_count = 1;
				I internal_buffer_begin = begin;
				I internal_buffer_end = begin + unique_count;
				for (I i = internal_buffer_end; unique_count != expected_unique_count && i != end; ++i)
				{
					I target = lower_bound<HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT | HYBRID_SEARCH_OPTIONS_BACKWARD_BIT>(internal_buffer_begin, internal_buffer_end, i);
					if (target == internal_buffer_end || *i != *target)
					{
						size_type offset = (i - unique_count) - internal_buffer_begin;
						if (i != internal_buffer_end)
							rotate(internal_buffer_begin, internal_buffer_end, i);
						internal_buffer_begin = std::next(internal_buffer_begin, offset);
						internal_buffer_end = std::next(internal_buffer_end, offset);
						target = std::next(target, offset);
						insert_backward(target, i);
						++internal_buffer_end;
						++unique_count;
					}
				}
				if (internal_buffer_begin != begin)
					rotate(begin, internal_buffer_begin, internal_buffer_end);
				return unique_count;
			}

			constexpr void merge_runs_forward(I begin, I middle, I end, I buffer_begin)
			{
				if (*std::prev(middle, 1) <= *middle)
					return;
				I left = begin;
				I right = middle;
				I buffer = buffer_begin;
				while (right < end)
				{
					bool flag = left == middle || *left > *right;
					I& target = flag ? right : left;
					std::iter_swap(buffer, target);
					++target;
					++buffer;
				}
				if (buffer != left)
					swap_range(buffer, buffer + distance(left, middle), left);
			}

			constexpr void merge_runs_backward(I begin, I middle, I end, I buffer_end)
			{
				if (*std::prev(middle, 1) <= *middle)
					return;
				I buffer = buffer_end - 1;
				I right = end - 1;
				I left = middle - 1;
				while (left >= begin)
				{
					bool flag = right < middle || *left > *right;
					I& target = flag ? left : right;
					std::iter_swap(buffer, target);
					--target;
					--buffer;
				}
				if (right == buffer)
					return;
				while (right >= middle)
				{
					std::iter_swap(buffer, right);
					--buffer;
					--right;
				}
			}

			constexpr void insertion_sort_basic(I begin, I end)
			{
				for (I i = begin + 1; i < end; ++i)
				{
					value_type tmp = std::move(*i);
					I j = i - 1;
					for (; j >= begin && *j > tmp; --j)
						iter_move(j + 1, j);
					iter_move(j + 1, tmp);
				}
			}

			constexpr void insertion_sort_unstable(I begin, I end)
			{
				if (distance(begin, end) <= BASIC_INSERTION_SORT_THRESHOLD)
					return insertion_sort_basic(begin, end);
				for (I i = begin + 1; i < end; ++i)
				{
					if (*i < *begin)
						std::iter_swap(begin, i);
					value_type tmp = std::move(*i);
					I j = i - 1;
					for (; *j > tmp; --j)
						iter_move(j + 1, j);
					iter_move(j + 1, tmp);
				}
			}

			constexpr void insertion_sort_stable(I begin, I end)
			{
				if (distance(begin, end) <= BASIC_INSERTION_SORT_THRESHOLD)
					return insertion_sort_basic(begin, end);
				I min = begin;
				for (I i = min + 1; i < end; ++i)
					if (*i < *min)
						min = i;
				value_type tmp = std::move(*min);
				for (I i = min; i > begin; --i)
					iter_move(i, i - 1);
				iter_move(begin, std::move(tmp));
				for (I i = begin + 1; i < end; ++i)
				{
					tmp = std::move(*i);
					I j = i - 1;
					for (; *j > tmp; --j)
						iter_move(j + 1, j);
					iter_move(j + 1, std::move(tmp));
				}
			}

			constexpr void sort_tags(I begin, I end)
			{
				insertion_sort_unstable(begin, end);
			}

			constexpr size_type sort_small_runs(I internal_buffer_end, I end)
			{
				size_type run_count = 0;
				while (true)
				{
					++run_count;
					I next = internal_buffer_end + SMALL_SORT_THRESHOLD;
					if (next > end)
					{
						if (next != end)
							insertion_sort_stable(internal_buffer_end, end);
						break;
					}
					insertion_sort_stable(internal_buffer_end, next);
					internal_buffer_end = next;
				}
				return run_count;
			}

			constexpr void lazy_merge(I begin, I middle, I end)
			{
				while (middle != end)
				{
					I target = upper_bound(begin, middle, end - 1);
					if (target != middle)
					{
						rotate(target, middle, end);
						middle = target;
					}
					if (begin == middle)
						break;
					do
					{
						--end;
					} while (middle != end && *(middle - 1) <= *(end - 1));
				}
			}

			constexpr void lazy_merge_last(I begin, I middle, I end)
			{
				while (begin != middle)
				{
					I target = lower_bound(middle, end, begin);
					if (target != middle)
					{
						rotate(begin, middle, target);
						begin += distance(middle, target);
					}
					if (middle == end)
						break;
					do
					{
						begin++;
					} while (begin != middle && *begin <= *middle);
				}
			}

			constexpr void lazy_merge_sort(I begin, I end)
			{
				sort_small_runs(begin, end);
				for (size_type run_size = SMALL_SORT_THRESHOLD; run_size < distance(begin, end); run_size *= 2)
				{
					for (I i = begin;;)
					{
						I merge_middle = i + run_size;
						if (merge_middle >= end)
							break;
						I merge_end = merge_middle + run_size;
						if (*std::prev(merge_middle, 1) > *merge_end)
						{
							if (merge_end > end)
							{
								lazy_merge_last(i, merge_middle, end);
								break;
							}
							lazy_merge(i, merge_middle, merge_end);
						}
						i = merge_end;
					}
				}
			}

			constexpr void fallback_sort(I begin, I end, size_type unique_count)
			{
				lazy_merge_sort(begin, end);
			}

			constexpr void internal_merge_pass_forward(I working_buffer, I begin, I end, size_type run_size)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin < end);

				while (true)
				{
					I merge_middle = begin + run_size;
					if (merge_middle >= end)
						break;
					I merge_end = merge_middle + run_size;
					if (merge_end > end)
						merge_end = end;
					merge_runs_forward(begin, merge_middle, merge_end, working_buffer);
					working_buffer = merge_middle;
					begin = merge_end;
				}
			}

			constexpr size_type internal_merge_pass_forward(I working_buffer, I begin, I end, size_type run_size, size_type run_count)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin < end);

				while (true)
				{
					I merge_middle = begin + run_size;
					if (merge_middle >= end)
						break;
					I merge_end = merge_middle + run_size;
					if (merge_end > end)
						merge_end = end;
					merge_runs_forward(begin, merge_middle, merge_end, working_buffer);
					--run_count;
					working_buffer = merge_middle;
					begin = merge_end;
				}
				return run_count;
			}

			constexpr void internal_merge_pass_backward(I begin, I end, I working_buffer_end, size_type run_size)
			{
				while (true)
				{
					I merge_middle = end - run_size;
					if (merge_middle <= begin)
					{
						if (merge_middle == begin)
							rotate(begin, end, working_buffer_end);
						break;
					}
					I merge_begin = merge_middle - run_size;
					if (merge_begin < begin)
						merge_begin = begin;
					merge_runs_backward(merge_begin, merge_middle, end, working_buffer_end);
					working_buffer_end = merge_middle;
					end = merge_begin;
				}
			}

			constexpr size_type internal_merge_pass_backward(I begin, I end, I working_buffer_end, size_type run_size, size_type run_count)
			{
				while (true)
				{
					I merge_middle = end - run_size;
					if (merge_middle <= begin)
					{
						if (merge_middle == begin)
							rotate(begin, end, working_buffer_end);
						break;
					}
					I merge_begin = merge_middle - run_size;
					if (merge_begin < begin)
						merge_begin = begin;
					merge_runs_backward(merge_begin, merge_middle, end, working_buffer_end);
					--run_count;
					working_buffer_end = merge_middle;
					end = merge_begin;
				}
				return run_count;
			}

			constexpr size_type merge_small_runs(I begin, I internal_buffer_end, I end, size_type internal_buffer_size, size_type run_count)
			{
				size_type rotated_count = 0;
				size_type run_size = SMALL_SORT_THRESHOLD;
				I array_end = end;
				I internal_buffer_end_tmp = internal_buffer_end;
				do
				{
					size_type next_run_size = run_size * 2;
					internal_buffer_end_tmp -= run_size;
					run_count = internal_merge_pass_forward(internal_buffer_end_tmp, internal_buffer_end, end, run_size, run_count);
					end -= run_size;
					internal_buffer_end -= run_size;
					rotated_count += run_size;
					run_size = next_run_size;
				} while (run_size < internal_buffer_size);
				size_type remaining = internal_buffer_size - rotated_count;
				if (remaining != 0)
				{
					I tmp = internal_buffer_end - remaining;
					rotate(tmp, internal_buffer_end, end);
					internal_buffer_end = tmp;
					end -= remaining;
				}
				if ((run_count & 1) == 0)
				{
					I new_end = end - run_size;
					swap_range(end, array_end, new_end);
					end = new_end;
					array_end -= run_size;
				}
				run_count = internal_merge_pass_backward(begin, end, array_end, run_size, run_count);
				return run_count;
			}

			constexpr void local_block_merge(I working_buffer, I begin, I middle, I end, bool left_origin, bool right_origin)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin < end);
				I middle_original = middle;
				while (begin != middle_original && middle != end)
				{
					I& rhs = (!left_origin ? *begin <= *middle : *begin < *middle) ? begin : middle;
					std::iter_swap(working_buffer, rhs);
					++rhs;
					++working_buffer;
				}
			}

			constexpr void block_merge_forward(I tag_buffer, I working_buffer, I begin, I end, I median_tag, size_type block_size)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin < end);
				size_type step = block_size * 2;
				I left_tag = tag_buffer;
				while (true)
				{
					I right_tag = left_tag + 1;
					I left_block = begin;
					if (begin >= end)
						break;
					I right_block = begin + block_size;
					I next = right_block + block_size;
					if (next > end)
						next = end;
					bool left_origin = *left_tag < *median_tag;
					bool right_origin = *right_block < *median_tag;
					if (left_origin == right_origin)
					{
						swap_range(left_block, right_block, next);
					}
					else
					{
						local_block_merge(working_buffer, left_block, right_block, next, left_origin, right_origin);
					}
					++left_tag;
					++right_tag;
					begin = next;
					working_buffer = right_block;
				}
			}

			constexpr void block_select_forward(I tag_buffer, I working_buffer, I begin, I middle, I end, size_type block_size)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin - block_size == working_buffer);
				GRAILSORT_INVARIANT(begin < end);
				sort_tags(tag_buffer, tag_buffer + block_size);
				I median_tag = std::next(tag_buffer, distance(tag_buffer, working_buffer) / 2);
				I this_block = begin;
				I this_tag = tag_buffer;
				do
				{
					I min_block = this_block;
					I min_tag = this_tag;
					I next_block = std::next(min_block, block_size);
					I next_tag = std::next(min_tag);
					I candidate_block = next_block;
					I cadidate_tag = next_tag;
					while (candidate_block < end)
					{
						bool flag = *candidate_block < *min_block;
						if (!flag)
							flag = *candidate_block == *min_block;
						if (!flag)
							flag = *cadidate_tag < *min_tag;
						if (flag)
						{
							min_tag = cadidate_tag;
							min_block = candidate_block;
						}
						candidate_block = std::next(candidate_block, block_size);
						cadidate_tag = std::next(cadidate_tag);
					}
					if (min_block != this_block)
					{
						swap_range(this_block, next_block, min_block);
						std::iter_swap(this_tag, min_tag);
						if (median_tag == this_tag)
							median_tag = min_tag;
						else if (median_tag == min_tag)
							median_tag = this_tag;
					}
					this_block = next_block;
					this_tag = next_tag;
				} while (this_block < end);
				block_merge_forward(tag_buffer, working_buffer, begin, this_block, median_tag, block_size);
			}

			constexpr void block_merge_pass_forward(I tag_buffer, I working_buffer, I begin, I end, size_type block_size, size_type run_size)
			{
				while (true)
				{
					I merge_begin = begin;
					I merge_middle = begin + run_size;
					if (merge_middle > end)
						break;
					I merge_end = merge_middle + run_size;
					if (merge_end > end)
						merge_end = end;
					block_select_forward(tag_buffer, working_buffer, merge_begin, merge_middle, merge_end, block_size);
					working_buffer += run_size * 2;
					begin = working_buffer + block_size;
				}
			}

			constexpr void insert_internal_buffer(I begin, I internal_buffer_end, I end)
			{
			}

			constexpr void sort_internal(I begin, I end)
			{
				size_type size = distance(begin, end);
				if (size <= SMALL_SORT_THRESHOLD)
					return insertion_sort_stable(begin, end);
				size_type size_sqrt = sqrt_of(size);
				size_type internal_buffer_size = size_sqrt * 2;
				size_type unique_count = gather_unique(begin, end, internal_buffer_size);
				if (unique_count == 1)
					return; // great
				if (unique_count != internal_buffer_size)
					return fallback_sort(begin, end, unique_count);
				I tag_buffer = begin;
				I working_buffer = std::next(begin, size_sqrt);
				I internal_buffer_end = begin + internal_buffer_size;
				size_type run_count = sort_small_runs(internal_buffer_end, end);
				merge_small_runs(begin, internal_buffer_end, end, internal_buffer_size, run_count);
				block_merge_pass_forward(tag_buffer, working_buffer, internal_buffer_end, end, size_sqrt, 4 * size_sqrt);
			}

			constexpr void sort_external(I begin, I end)
			{

			}
		};
	}

	template <typename RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin, RandomAccessIterator end)
	{
		detail::helper_type<RandomAccessIterator, void>().sort_internal(begin, end);
	}

	template <typename RandomAccessIterator, typename BufferIterator = RandomAccessIterator>
	constexpr void sort(
		RandomAccessIterator begin, RandomAccessIterator end,
		BufferIterator external_buffer_begin, BufferIterator external_buffer_end)
	{
		detail::helper_type<RandomAccessIterator, BufferIterator> state = {};
		state.external_buffer_begin = external_buffer_begin;
		state.external_buffer_end = external_buffer_end;
		state.sort_external(begin, end);
	}
}

void grailsort_cpp(main_array array)
{
	grailsort::sort(array.begin(), array.end());
}