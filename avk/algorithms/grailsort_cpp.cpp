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
	This file contains a C++17 implementation of Andrei Astrelin's GrailSort, a block merge sorting algorithm, with some changes/optimizations.
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
#define GRAILSORT_INVARIANT(...) assert(__VA_ARGS__)
#else
#define GRAILSORT_INVARIANT(...)
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

		template <typename I, typename J>
		struct grail_sort_helper : grail_sort_external_buffer_helper_type<J>
		{
			using iterator_traits = std::iterator_traits<I>;
			using value_type = typename iterator_traits::value_type;
			using difference_type = typename iterator_traits::difference_type;
			using size_type = std::make_unsigned_t<difference_type>;

			static constexpr bool using_external_buffer = !std::is_void<J>::value;

			static constexpr uint_fast8_t log2_of(size_t size) noexcept
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
				while (size != 0)
					size >>= 1;
				return (size & 1) != 0;
			}

			static constexpr bool is_pow4(size_type size)
			{
				return (log2_of(size) & 1) != 0;
			}

			struct internal
			{
				static constexpr size_t
					CACHE_LINE_SIZE					= std::hardware_constructive_interference_size,
					SMALL_SORT_THRESHOLD			= std::max<size_t>(CACHE_LINE_SIZE / sizeof(value_type), 8),
					SMALL_SORT_THRESHOLD_SQRT		= sqrt_of(SMALL_SORT_THRESHOLD),
					SMALL_SORT_THRESHOLD_LOG2		= log2_of(SMALL_SORT_THRESHOLD),
					MEDIUM_SORT_THRESHOLD			= SMALL_SORT_THRESHOLD * 4,
					LINEAR_SEARCH_THRESHOLD			= SMALL_SORT_THRESHOLD,
					BASIC_INSERTION_SORT_THRESHOLD	= 4,
					GATHER_UNIQUE_BASIC_THRESHOLD	= CACHE_LINE_SIZE;

				static_assert(CACHE_LINE_SIZE <= std::numeric_limits<size_type>::max());
				static_assert(SMALL_SORT_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(SMALL_SORT_THRESHOLD_SQRT <= std::numeric_limits<size_type>::max());
				static_assert(SMALL_SORT_THRESHOLD_LOG2 <= std::numeric_limits<size_type>::max());
				static_assert(MEDIUM_SORT_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(LINEAR_SEARCH_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(BASIC_INSERTION_SORT_THRESHOLD <= std::numeric_limits<size_type>::max());
				static_assert(GATHER_UNIQUE_BASIC_THRESHOLD <= std::numeric_limits<size_type>::max());
			};

			static constexpr size_type
				CACHE_LINE_SIZE					= static_cast<size_type>(internal::CACHE_LINE_SIZE),
				SMALL_SORT_THRESHOLD			= static_cast<size_type>(internal::SMALL_SORT_THRESHOLD),
				SMALL_SORT_THRESHOLD_SQRT		= static_cast<size_type>(internal::SMALL_SORT_THRESHOLD_SQRT),
				SMALL_SORT_THRESHOLD_LOG2		= static_cast<size_type>(internal::SMALL_SORT_THRESHOLD_SQRT),
				MEDIUM_SORT_THRESHOLD			= static_cast<size_type>(internal::MEDIUM_SORT_THRESHOLD),
				LINEAR_SEARCH_THRESHOLD			= static_cast<size_type>(internal::LINEAR_SEARCH_THRESHOLD),
				BASIC_INSERTION_SORT_THRESHOLD	= static_cast<size_type>(internal::BASIC_INSERTION_SORT_THRESHOLD),
				GATHER_UNIQUE_BASIC_THRESHOLD	= static_cast<size_type>(internal::GATHER_UNIQUE_BASIC_THRESHOLD);

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
					if (left_size <= right_size)
					{
						swap_range(begin, begin + left_size, begin + left_size);
						begin += left_size;
						right_size -= left_size;
					}
					else
					{
						swap_range(begin + (left_size - right_size), begin + left_size, begin + left_size);
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

			using hybrid_search_options = uint_fast8_t;
			static constexpr uint_fast8_t
				HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT = 1,
				HYBRID_SEARCH_OPTIONS_BACKWARD_BIT = 2;

			template <hybrid_search_options Options, typename P>
			static constexpr I hybrid_search(I begin, I end, I key, P&& compare)
			{
				constexpr bool all_unique = (Options & HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT) != 0;
				constexpr bool backward = (Options & HYBRID_SEARCH_OPTIONS_BACKWARD_BIT) != 0;

				if (distance(begin, end) <= LINEAR_SEARCH_THRESHOLD)
					return linear_search_forward(begin, end, key, compare);

				I i = begin;
				begin += LINEAR_SEARCH_THRESHOLD;
				while (i != begin)
				{
					if (!compare(i, key))
						return i;
					++i;
				}

				if (begin == end)
					return begin;

				i = begin;
				size_type increment = 1;
				while (i < end)
				{
					if constexpr (all_unique)
					{
						if (*i == *key)
							return i;
					}
					if (!compare(i, key))
						break;
					i += increment;
					increment += increment;
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
						begin = i + 1;
					else
						end = i;
				}

				return linear_search_forward(begin, end, key, compare);
			}

			template <hybrid_search_options Options = 0>
			static constexpr I lower_bound(I begin, I end, I key)
			{
				return hybrid_search<Options>(begin, end, key, [](I left, I right)
				{
					return *left < *right;
				});
			}

			template <hybrid_search_options Options = 0>
			static constexpr I upper_bound(I begin, I end, I key)
			{
				return hybrid_search<Options>(begin, end, key, [](I left, I right)
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

				for (I i = internal_buffer_end; unique_count != expected_unique_count && i != end; ++i)
				{
					constexpr auto options =
						HYBRID_SEARCH_OPTIONS_ALL_UNIQUE_BIT |
						HYBRID_SEARCH_OPTIONS_BACKWARD_BIT;

					I target = lower_bound<options>(internal_buffer_begin, internal_buffer_end, i);
					if (target == internal_buffer_end || *i != *target)
					{
						const size_type offset = (i - unique_count) - internal_buffer_begin;
						rotate(internal_buffer_begin, internal_buffer_end, i);
						internal_buffer_begin += offset;
						internal_buffer_end += offset;
						target += offset;
						insert_backward(target, i);
						++internal_buffer_end;
						++unique_count;
					}
				}

				rotate(begin, internal_buffer_begin, internal_buffer_end);
				return unique_count;
			}

			static constexpr void internal_merge_forward(I begin, I middle, I end, I buffer_begin)
			{
				I left = begin;
				I right = middle;
				I buffer = buffer_begin;

				while (right < end)
				{
					if (left == middle || *left > *right)
					{
						std::iter_swap(buffer, right);
						++right;
					}
					else
					{
						std::iter_swap(buffer, left);
						++left;
					}
					++buffer;
				}

				if (buffer != left)
					swap_range(buffer, buffer + distance(left, middle), left);
			}

			static constexpr void internal_merge_backward(I begin, I middle, I end, I buffer_end)
			{
				I buffer = buffer_end - 1;
				I right = end - 1;
				I left = middle - 1;

				while (left >= begin)
				{
					if (right < middle || *left > *right)
					{
						std::iter_swap(buffer, left);
						--left;
					}
					else
					{
						std::iter_swap(buffer, right);
						--right;
					}
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

			static constexpr void external_merge_forward(I begin, I middle, I end)
			{
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

			static constexpr void build_small_runs(I internal_buffer_end, I end)
			{
				while (true)
				{
					const I next = internal_buffer_end + SMALL_SORT_THRESHOLD;
					if (next > end)
						return insertion_sort_stable(internal_buffer_end, end);
					insertion_sort_stable(internal_buffer_end, next);
					internal_buffer_end = next;
				}
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
						return lazy_merge_last(begin, merge_middle, end);
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

			static constexpr void build_large_runs(I begin, I internal_buffer_end, I end, size_type internal_buffer_size)
			{
				size_type rotated_count = 0;
				size_type run_size = SMALL_SORT_THRESHOLD;

				const I original_end = end;
				I internal_buffer_end_tmp = internal_buffer_end;

				while (true)
				{
					const size_type next_run_size = run_size * 2;
					internal_buffer_end_tmp -= run_size;
					internal_merge_pass_forward(internal_buffer_end_tmp, internal_buffer_end, end, run_size);
					end -= run_size;
					internal_buffer_end -= run_size;
					rotated_count += run_size;
					run_size = next_run_size;
					if (run_size >= internal_buffer_size)
						break;
				}

				size_type remaining = internal_buffer_size - rotated_count;
				if (remaining != 0)
				{
					const I tmp = internal_buffer_end - remaining;
					rotate(tmp, internal_buffer_end, end);
					internal_buffer_end = tmp;
					end -= remaining;
				}

				internal_merge_pass_backward(begin, end, original_end, run_size);
			}

			static constexpr void tagged_swap(I from_tag, I to_tag, I from_begin, I from_end, I to_begin)
			{
				iter_swap(from_tag, to_tag);
				swap_range(from_begin, from_end, to_begin);
			}

			static constexpr void block_merge(I tag_buffer, I working_buffer, I begin, I end, size_type block_size)
			{
				size_type block_size_log2 = log2_of(block_size);

				sort_tag_buffer(tag_buffer, working_buffer);

				const I middle = begin + distance(begin, end) / 2;
				const I original_begin = begin;

				while (true)
				{
					if (begin == middle)
						return;
					if (*begin > *middle)
						break;
					++tag_buffer;
					begin += block_size;
				}

				while (begin != end)
				{
					I min = begin;
					I i = begin;
					do
					{
						i += block_size;
						if (*i < *min)
							min = i;
					} while (i != end);
					tagged_swap(tag_buffer, tag_buffer + (distance(begin, min) >> block_size_log2), begin, begin + block_size, min);
					++tag_buffer;
					begin += block_size;
				}

				internal_merge_pass_forward(working_buffer, begin, end, block_size);
			}

			static constexpr void block_merge_sort(I tag_buffer, I working_buffer, I begin, I end, size_type block_size)
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
				if (unique_count != internal_buffer_size)
					return lazy_merge_sort(begin, end);
				const I tag_buffer = begin;
				const I working_buffer = begin + size_sqrt;
				const I internal_buffer_end = begin + internal_buffer_size;
				build_small_runs(internal_buffer_end, end);
				build_large_runs(begin, internal_buffer_end, end, internal_buffer_size);
				return;
				block_merge(tag_buffer, working_buffer, internal_buffer_end, internal_buffer_end + 2 * internal_buffer_size, sqrt_of(internal_buffer_size));
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

void grail_sort_cpp(main_array array)
{
	grailsort::sort(array.begin(), array.end());
}