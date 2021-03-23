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
#define GRAILSORT_DEBUG
#endif

#ifdef GRAILSORT_DEBUG
#include <cassert>
#define GRAILSORT_INVARIANT(expression) assert(expression)
#else
#define GRAILSORT_INVARIANT(expression)
#endif

namespace grailsort
{
	namespace detail
	{
		template <typename J>
		struct grail_sort_external_buffer_helper_type
		{
			J external_buffer_begin;
			J external_buffer_end;

			constexpr void assign_external_buffer(J begin, J end)
			{
				external_buffer_begin = begin;
				external_buffer_end = end;
			}
		};

		template <>
		struct grail_sort_external_buffer_helper_type<void>
		{
		};

		template <
			typename I, // RandomAccessIterator
			typename J> // BufferIterator
		struct grail_sort_helper : grail_sort_external_buffer_helper_type<J>
		{
			using iterator_traits = std::iterator_traits<I>;
			using value_type = typename iterator_traits::value_type;
			using difference_type = typename iterator_traits::difference_type;
			using size_type = std::make_unsigned_t<difference_type>;

			static constexpr bool using_external_buffer = !std::is_void<J>::value;
			static constexpr bool iterator_cmovable = sizeof(I) <= sizeof(void*) && std::is_trivially_copyable<I>::value;

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

			static constexpr bool is_even(size_type size)
			{
				if constexpr (std::is_fundamental<size_type>::value && std::is_integral<size_type>::value)
					return (size & 1) == 0;
				else
					return (size % 2) == 0;
			}

			static constexpr bool is_pow2(size_type size)
			{
				while (size != 0)
					size >>= 1;
				return (size & 1) != 0;
			}

			struct checked_thresholds
			{
				static constexpr size_t
					CACHE_LINE_SIZE					= std::hardware_constructive_interference_size,
					SMALL_SORT_THRESHOLD			= std::max<size_t>(CACHE_LINE_SIZE / sizeof(value_type), 8),
					SMALL_SORT_THRESHOLD_LOG2		= log2_of(SMALL_SORT_THRESHOLD),
					MEDIUM_SORT_THRESHOLD			= SMALL_SORT_THRESHOLD * 4,
					LINEAR_SEARCH_THRESHOLD			= SMALL_SORT_THRESHOLD,
					BASIC_INSERTION_SORT_THRESHOLD	= 6,
					GATHER_UNIQUE_BASIC_THRESHOLD	= CACHE_LINE_SIZE / sizeof(value_type);

				static_assert(CACHE_LINE_SIZE <= std::numeric_limits<size_type>::max());
				static_assert(SMALL_SORT_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(SMALL_SORT_THRESHOLD_LOG2 <= std::numeric_limits<size_type>::max());
				static_assert(MEDIUM_SORT_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(LINEAR_SEARCH_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(BASIC_INSERTION_SORT_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(GATHER_UNIQUE_BASIC_THRESHOLD <= std::numeric_limits<size_type>::max());
			};

			static constexpr size_type
				EXPECTED_PAGE_SIZE				= static_cast<size_type>(8192),
				CACHE_LINE_SIZE					= static_cast<size_type>(checked_thresholds::CACHE_LINE_SIZE),
				SMALL_SORT_THRESHOLD			= static_cast<size_type>(checked_thresholds::SMALL_SORT_THRESHOLD),
				SMALL_SORT_THRESHOLD_LOG2		= static_cast<size_type>(checked_thresholds::SMALL_SORT_THRESHOLD_LOG2),
				MEDIUM_SORT_THRESHOLD			= static_cast<size_type>(checked_thresholds::MEDIUM_SORT_THRESHOLD),
				LINEAR_SEARCH_THRESHOLD			= static_cast<size_type>(checked_thresholds::LINEAR_SEARCH_THRESHOLD),
				BASIC_INSERTION_SORT_THRESHOLD	= static_cast<size_type>(checked_thresholds::BASIC_INSERTION_SORT_THRESHOLD),
				GATHER_UNIQUE_BASIC_THRESHOLD	= static_cast<size_type>(checked_thresholds::GATHER_UNIQUE_BASIC_THRESHOLD);

			static constexpr size_type distance(I begin, I end)
			{
				GRAILSORT_INVARIANT(begin <= end);

				return (size_type)std::distance(begin, end);
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

			static constexpr void iter_swap(I left, I right)
			{
				std::iter_swap(left, right);
			}

			template <typename I2 = I>
			static constexpr void swap_range(I from_begin, I from_end, I2 to_begin)
			{
				GRAILSORT_INVARIANT(from_begin <= from_end);

				while (from_begin != from_end)
				{
					iter_swap(from_begin, to_begin);
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

			static constexpr void insert_backward(I target, I from)
			{
				GRAILSORT_INVARIANT(target <= from);

				value_type tmp = std::move(*from);
				--from;
				for (; from >= target; --from)
					iter_move(from + 1, from);
				iter_move(target, tmp);
			}

			static constexpr void rotate(I begin, I new_begin, I end)
			{
				GRAILSORT_INVARIANT(begin <= end);
				GRAILSORT_INVARIANT(new_begin <= end);
				GRAILSORT_INVARIANT(begin <= new_begin);

				size_type left_size = distance(begin, new_begin);
				size_type right_size = distance(new_begin, end);
				while (left_size != 0 && right_size != 0)
				{
					const I middle = std::next(begin, left_size);
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
			static constexpr I linear_search_forward(I begin, I end, I key, P&& compare)
			{
				while (begin != end && compare(begin, key))
					++begin;
				return begin;
			}

			using hybrid_search_flags = uint_fast8_t;
			static constexpr uint_fast8_t
				HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT = 1,
				HYBRID_SEARCH_OPTIONS_BACKWARD_BIT = 2;

			template <hybrid_search_flags Flags, typename P>
			static constexpr I hybrid_search(I begin, I end, I key, P&& compare)
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
					{
						if (*i == *key)
							return i;
					}
					if (!compare(i, key))
						break;
					i = std::next(i, increment);
				}

				if (i > end)
					i = end;

				while (true)
				{
					const size_type d = distance(begin, end);
					const I i = std::next(begin, d / 2);
					if constexpr (all_unique)
					{
						if (*i == *key)
							return i;
					}
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
			static constexpr I lower_bound(I begin, I end, I key)
			{
				return hybrid_search<Flags>(begin, end, key, [](I left, I right)
				{
					return *left < *right;
				});
			}

			template <hybrid_search_flags Flags = 0>
			static constexpr I upper_bound(I begin, I end, I key)
			{
				return hybrid_search<Flags>(begin, end, key, [](I left, I right)
				{
					return !(*left > *right);
				});
			}

			// maybe delete this:
			static constexpr size_type gather_unique_basic(I begin, I end, size_type expected_unique_count)
			{
				size_type unique_count = 1;
				I internal_buffer_end = begin + 1;
				for (I i = internal_buffer_end; i != end && unique_count != expected_unique_count; ++i)
				{
					constexpr auto options =
						HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT |
						HYBRID_SEARCH_OPTIONS_BACKWARD_BIT;

					const I target = lower_bound<options>(begin, internal_buffer_end, i);
					if (target == internal_buffer_end || *i != *target)
					{
						insert_backward(target, i);
						++unique_count;
						++internal_buffer_end;
					}
				}
				return unique_count;
			}

			static constexpr size_type gather_unique(I begin, I end, size_type expected_unique_count)
			{
				if (distance(begin, end) <= GATHER_UNIQUE_BASIC_THRESHOLD)
					return gather_unique_basic(begin, end, expected_unique_count);

				size_type unique_count = 1;
				I internal_buffer_begin = begin;
				I internal_buffer_end = begin + unique_count;

				constexpr auto options =
					HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT |
					HYBRID_SEARCH_OPTIONS_BACKWARD_BIT;

				for (I i = internal_buffer_end; unique_count != expected_unique_count && i != end; ++i)
				{
					I target = lower_bound<options>(internal_buffer_begin, internal_buffer_end, i);
					if (target == internal_buffer_end || *i != *target)
					{
						const size_type offset = (i - unique_count) - internal_buffer_begin;
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

			static constexpr void internal_merge_forward(I begin, I middle, I end, I buffer_begin)
			{
				if (*std::prev(middle, 1) <= *middle)
					return;

				I left = begin;
				I right = middle;
				I buffer = buffer_begin;

				//Note: attempt to unroll this?
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

			static constexpr void internal_merge_forward_origin_aware(I working_buffer, I begin, I middle, I end, bool origin)
			{
				I middle_original = middle;
				while (begin != middle_original && middle != end)
				{
					const bool flag = origin ?
						*begin <= *middle :
						*begin < *middle;

					if (flag)
					{
						std::iter_swap(working_buffer, begin);
						++begin;
					}
					else
					{
						std::iter_swap(working_buffer, middle);
						++middle;
					}

					++working_buffer;
				}
			}

			static constexpr void internal_merge_backward(I begin, I middle, I end, I buffer_end)
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

			static constexpr void insertion_sort_basic(I begin, I end)
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

			static constexpr void insertion_sort_unstable(I begin, I end)
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

			static constexpr void insertion_sort_stable(I begin, I end)
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

			static constexpr void sort_tag_buffer(I begin, I end)
			{
				insertion_sort_unstable(begin, end);
			}

			static constexpr void sort_tag_buffer(I begin, I end, I median)
			{
				I lesser = begin;
				I greater = end;

			}

			static constexpr size_type build_small_runs(I internal_buffer_end, I end)
			{
				size_type run_count = 0;
				while (true)
				{
					++run_count;
					const I next = internal_buffer_end + SMALL_SORT_THRESHOLD;
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

			static constexpr void lazy_merge(I begin, I middle, I end)
			{
				while (middle != end)
				{
					const I target = upper_bound(begin, middle, end - 1);
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

			static constexpr void lazy_merge_last(I begin, I middle, I end)
			{
				while (begin != middle)
				{
					const I target = lower_bound(middle, end, begin);
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

			static constexpr void lazy_merge_pass(I begin, I end, size_type run_size)
			{
				while (true)
				{
					const I merge_middle = begin + run_size;
					if (merge_middle >= end)
						return;
					const I merge_end = merge_middle + run_size;
					if (merge_end > end)
						if (*std::prev(merge_middle, 1) > *merge_end)
							return lazy_merge_last(begin, merge_middle, end);
					if (*std::prev(merge_middle, 1) > *merge_end)
						lazy_merge(begin, merge_middle, merge_end);
					begin = merge_end;
				}
			}

			static constexpr void lazy_merge_sort(I begin, I end)
			{
				build_small_runs(begin, end);
				for (size_type run_size = SMALL_SORT_THRESHOLD; run_size < distance(begin, end); run_size *= 2)
					lazy_merge_pass(begin, end, run_size);
			}

			static constexpr void encode_zero(I begin, uint8_t bits)
			{
				insertion_sort_unstable(begin, std::next(begin, bits));
			}

			static constexpr void encode_value(I begin, uint8_t bits, size_type value)
			{
				while (value != 0)
				{
					if (value & 1)
						iter_swap(begin, std::next(begin, 1));
					value >>= 1;
					begin = std::next(begin, 2);
				}
			}

			static constexpr size_type decode_value(I begin, uint8_t bits)
			{
				size_type r = 0;
				for (uint8_t i = 0; i != bits; ++i)
					if (*begin > *std::next(begin, 1))
						r |= (1 << i);
				return r;
			}

			static constexpr void fallback_sort(I begin, I end, size_type unique_count)
			{
				auto prior_begin = begin;
				uint8_t desired = 2 * log2_of(distance(begin, end));
				uint8_t n = 1;
				begin += unique_count;
				while (n != unique_count)
				{
					size_type found = gather_unique(begin, end, desired);
					if (found < desired)
						return lazy_merge_sort(prior_begin, end);
					begin += desired;
				}
			}

			static constexpr void internal_merge_pass_forward(I working_buffer, I begin, I end, size_type run_size)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin < end);

				while (true)
				{
					//const I merge_begin = begin; //IMPLICIT
					const I merge_middle = begin + run_size;
					if (merge_middle >= end)
						break;
					I merge_end = merge_middle + run_size;
					if (merge_end > end)
						merge_end = end;
					internal_merge_forward(begin, merge_middle, merge_end, working_buffer);
					working_buffer = merge_middle;
					begin = merge_end;
				}
			}

			static constexpr size_type internal_merge_pass_forward(I working_buffer, I begin, I end, size_type run_size, size_type run_count)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin < end);

				while (true)
				{
					//const I merge_begin = begin; //IMPLICIT
					const I merge_middle = begin + run_size;
					if (merge_middle >= end)
						break;
					I merge_end = merge_middle + run_size;
					if (merge_end > end)
						merge_end = end;
					internal_merge_forward(begin, merge_middle, merge_end, working_buffer);
					--run_count;
					working_buffer = merge_middle;
					begin = merge_end;
				}
				return run_count;
			}

			static constexpr void internal_merge_pass_backward(I begin, I end, I working_buffer_end, size_type run_size)
			{
				while (true)
				{
					const I merge_middle = end - run_size;
					if (merge_middle <= begin)
					{
						if (merge_middle == begin)
							rotate(begin, end, working_buffer_end);
						break;
					}
					I merge_begin = merge_middle - run_size;
					if (merge_begin < begin)
						merge_begin = begin;
					internal_merge_backward(merge_begin, merge_middle, end, working_buffer_end);
					working_buffer_end = merge_middle;
					end = merge_begin;
				}
			}

			static constexpr size_type internal_merge_pass_backward(I begin, I end, I working_buffer_end, size_type run_size, size_type run_count)
			{
				while (true)
				{
					const I merge_middle = end - run_size;
					if (merge_middle <= begin)
					{
						if (merge_middle == begin)
							rotate(begin, end, working_buffer_end);
						break;
					}
					I merge_begin = merge_middle - run_size;
					if (merge_begin < begin)
						merge_begin = begin;
					internal_merge_backward(merge_begin, merge_middle, end, working_buffer_end);
					--run_count;
					working_buffer_end = merge_middle;
					end = merge_begin;
				}
				return run_count;
			}

			static constexpr size_type build_large_runs(I begin, I internal_buffer_end, I end, size_type internal_buffer_size, size_type run_count)
			{
				// BUILD RUNS USING INTERNAL BUFFER

				size_type rotated_count = 0;
				size_type run_size = SMALL_SORT_THRESHOLD;
				I array_end = end;
				I internal_buffer_end_tmp = internal_buffer_end;

				do
				{
					const size_type next_run_size = run_size * 2;
					internal_buffer_end_tmp -= run_size;
					run_count = internal_merge_pass_forward(internal_buffer_end_tmp, internal_buffer_end, end, run_size, run_count);
					end -= run_size;
					internal_buffer_end -= run_size;
					rotated_count += run_size;
					run_size = next_run_size;
				} while (run_size < internal_buffer_size);

				// BRING REMAINING INTERNAL BUFFER ELEMENTS TO THE END

				const size_type remaining = internal_buffer_size - rotated_count;
				if (remaining != 0)
				{
					const I tmp = internal_buffer_end - remaining;
					rotate(tmp, internal_buffer_end, end);
					internal_buffer_end = tmp;
					end -= remaining;
				}

				if (is_even(run_count))
				{
					// ADJUST INTERNAL BUFFER POSITION

					I new_end = end - run_size;
					swap_range(end, array_end, new_end);
					end = new_end;
					array_end -= run_size;
				}

				// PERFORM FINAL BACKWARDS MERGE PASS

				run_count = internal_merge_pass_backward(begin, end, array_end, run_size, run_count);

				// RUN SIZE IS NOW 4*SQRT(N)
				return run_count;
			}

			static constexpr void block_merge_forward(I tag_buffer, I working_buffer, I begin, I end, I median_tag, size_type block_size)
			{
				I left_tag = tag_buffer;
				I left_block = begin;
				bool left_origin = *left_tag < *median_tag;
				while (left_block < end)
				{
					I right_tag = left_tag + 1;
					I right_block = left_block + block_size;
					bool right_origin = *right_block < *median_tag;
					if (left_origin == right_origin)
					{
						swap_range(working_buffer, left_block, left_block);
					}
					else
					{
						internal_merge_forward_origin_aware(working_buffer, left_block, right_block, right_block + block_size, right_origin);
					}

					left_tag = right_tag;
					working_buffer = left_block;
					left_block = right_block;
				}
			}

			static constexpr void block_select_forward(I tag_buffer, I working_buffer, I begin, I middle, I end, size_type block_size)
			{
				GRAILSORT_INVARIANT(working_buffer < begin);
				GRAILSORT_INVARIANT(begin - block_size == working_buffer);
				GRAILSORT_INVARIANT(begin < end);

				sort_tag_buffer(tag_buffer, tag_buffer + block_size);

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
						iter_swap(this_tag, min_tag);
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

			static constexpr void block_merge_backward(I tag_buffer, I working_buffer, I begin, I middle, I end, size_type block_size)
			{
				internal_merge_pass_forward(working_buffer, begin, end, block_size);
			}

			static constexpr void block_merge_pass_forward(I tag_buffer, I working_buffer, I begin, I end, size_type block_size, size_type run_size)
			{
				while (true)
				{
					const I merge_begin = begin;
					const I merge_middle = begin + run_size;
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

			static constexpr void block_merge_pass_backward(I tag_buffer, I working_buffer, I begin, I end, size_type block_size, size_type run_size)
			{
			}

			static constexpr void insert_internal_buffer(I begin, I internal_buffer_end, I end)
			{
			}

			static constexpr void grail_sort_internal(I begin, I end)
			{
				const size_type size = distance(begin, end);
				if (size <= SMALL_SORT_THRESHOLD)
					return insertion_sort_stable(begin, end);
				const size_type size_sqrt = sqrt_of(size);
				const size_type internal_buffer_size = size_sqrt * 2;
				const size_type unique_count = gather_unique(begin, end, internal_buffer_size);
				if (unique_count == 1)
					return; // ??????????????
				if (unique_count != internal_buffer_size)
					return fallback_sort(begin, end, unique_count);
				const I tag_buffer = begin;
				const I working_buffer = std::next(begin, size_sqrt);
				const I internal_buffer_end = begin + internal_buffer_size;
				const size_type run_count = build_small_runs(internal_buffer_end, end);
				build_large_runs(begin, internal_buffer_end, end, internal_buffer_size, run_count);
				block_merge_pass_forward(tag_buffer, working_buffer, internal_buffer_end, end, size_sqrt, 4 * size_sqrt);
			}
		};
	}

	template <typename RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin, RandomAccessIterator end)
	{
		using helper_type = detail::grail_sort_helper<RandomAccessIterator, void>;
		helper_type::grail_sort_internal(begin, end);
	}

	template <typename RandomAccessIterator, typename BufferIterator = RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin, RandomAccessIterator end, BufferIterator external_buffer_begin, BufferIterator external_buffer_end)
	{
		using helper_type = detail::grail_sort_helper<RandomAccessIterator, BufferIterator>;
		helper_type state = {};
		if constexpr (helper_type::using_external_buffer)
			state.assign_external_buffer(external_buffer_begin, external_buffer_end);
		state.grail_sort_external(begin, end, external_buffer_begin, external_buffer_end);
	}
}

void grailsort_cpp(main_array array)
{
	grailsort::sort(array.begin(), array.end());
}