#include "all.h"

/*
	MIT License

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

#include <type_traits>
#include <limits>
#include <iterator>
#include <new>
#include <algorithm>



#if defined(_DEBUG) || !defined(NDEBUG)
#define MONITOR_SORT_DEBUG
#endif

#ifdef _MSVC_LANG
#define MONITOR_SORT_INLINE_ALWAYS __forceinline
#define MONITOR_SORT_INLINE_NEVER __declspec(noinline)
#define MONITOR_SORT_ASSUME(expression) __assume(expression)
#define MONITOR_SORT_UNREACHABLE __assume(0)
#endif

#ifdef MONITOR_SORT_DEBUG
#include <cassert>
#define MONITOR_SORT_INVARIANT(...) assert(__VA_ARGS__)
#else
#define MONITOR_SORT_INVARIANT(...) MONITOR_SORT_ASSUME(__VA_ARGS__)
#endif



namespace monitor_sort
{
	namespace platform
	{
		constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
		constexpr size_t page_size_hint = 4096;
		constexpr size_t large_page_size_hint = 1 << 21;
	}

#ifdef WSORT_PROFILE
	namespace profile
	{
		inline thread_local size_t swap_count;
		inline thread_local size_t comparisson_count;
	}
#endif

	template <typename T, size_t N>
	using array_type = T[N];

	template <typename T, size_t N>
	using array_ref = array_type<T, N>&;

	template <typename I>
	using value_type_of = typename std::iterator_traits<I>::value_type;

	template <typename I>
	using difference_type_of = typename std::iterator_traits<I>::difference_type;

	template <typename I>
	using size_type_of = typename std::make_unsigned<difference_type_of<I>>::type;

	template <size_t K>
	using min_uint_of =
		std::conditional_t<(K > UINT16_MAX),
			std::conditional_t<(K > UINT32_MAX), uint64_t, uint32_t>,
			std::conditional_t<(K > UINT8_MAX), uint16_t, uint8_t>>;

	template <typename T>
	struct sort_traits
	{
		struct type_info
		{
			enum : bool
			{
				trivial_move = std::is_trivially_move_constructible<T>::value,
				fundamental = std::is_fundamental<T>::value,
			};
		};

		enum : size_t
		{
			exponential_search_threshold = platform::large_page_size_hint / sizeof(T),

			binary_search_threshold = platform::page_size_hint / sizeof(T),

			linear_search_threshold =
				type_info::trivial_move ?
					type_info::fundamental ?
						(2 * platform::cache_line_size / sizeof(T)) :
						32 :
					8,
		};

		enum : uint_fast8_t
		{
			min_run_size = type_info::trivial_move ? 16 : 8,

			classic_insertion_sort_threshold = type_info::trivial_move ? 10 : 4,

			insertion_sort_threshold = type_info::trivial_move ? 64 : 16,

			small_sort_threshold_sqrt = type_info::trivial_move ? 8 : 4
		};
	};

	template <typename I>
	using traits_type_of = sort_traits<value_type_of<I>>;

	namespace bit
	{
		template <typename T, typename U>
		constexpr bool test(T mask, U index)
		{
			return (mask & ((T)1 << index)) != 0;
		}

		template <typename T, typename U>
		constexpr void set(T& mask, U index)
		{
			mask |= ((T)1 << index);
		}
		
		template <typename T, typename U>
		constexpr void reset(T& mask, U index)
		{
			mask &= ~((T)1 << index);
		}
	}

	template <typename T>
	constexpr T floor_to_even(T value)
	{
		value &= ~(T)1;
		return value;
	}

	template <typename T>
	constexpr T sqrt_of(T size)
	{
		T r = sort_traits<T>::small_sort_threshold_sqrt;
		while (r * r < size)
			r *= 2;
		return r;
	}

	template <typename T>
	constexpr uint_fast8_t log2_of(T size)
	{
		MONITOR_SORT_INVARIANT(size != 0);
		constexpr auto MAX_BITS = sizeof(T) * 8;
		for (uint_fast8_t i = MAX_BITS - 1; i > 0; --i)
			if (bit::test(size, i))
				return i;
		MONITOR_SORT_UNREACHABLE;
	}

	template <typename T>
	constexpr void swap(T& left, T& right)
	{
		std::swap(left, right);
	}

	template <typename T>
	constexpr void move(T& left, T& right)
	{
		new (&left) T(std::move(right));
	}

	template <typename I, typename T>
	constexpr void indirect_move(I left, T& right)
	{
		move(*left, right);
	}

	template <typename I>
	constexpr void indirect_move(I left, I right)
	{
		move(*left, *right);
	}

	template <typename I, typename O = I>
	constexpr void move_range(I from_begin, I from_end, O out_begin)
	{
		std::move(from_begin, from_end, out_begin);
	}

	template <typename K = size_t, typename I>
	constexpr auto unsigned_distance(I begin, I end)
	{
		auto r = std::distance(begin, end);
		MONITOR_SORT_INVARIANT(r >= 0);
		return (K)r;
	}

	template <typename I, typename O = I>
	constexpr void swap_range_forward(I left_begin, I left_end, O right_begin)
	{
		while (left_begin != left_end)
		{
			std::iter_swap(left_begin, right_begin);
			++left_begin;
			++right_begin;
		}
	}

	template <typename I, typename O = I>
	constexpr void swap_range_backward(I left_begin, I left_end, O right_end)
	{
		do
		{
			--left_end;
			--right_end;
			std::iter_swap(left_end, right_end);
		} while (left_end >= left_begin);
	}

	template <typename I>
	constexpr void reverse(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		--end;
		while (begin < end)
		{
			std::iter_swap(begin, end);
			--end;
			++begin;
		}
	}

	template <typename I>
	constexpr bool reverse_stable_initial_pass(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		--end;

		while (begin < end)
		{
			if (*begin == *end)
				break;
			std::iter_swap(begin, end);
			--end;
			++begin;
		}

		if (begin >= end)
			return true;

		while (begin < end)
		{
			std::iter_swap(begin, end);
			--end;
			++begin;
		}

		return false;
	}

	template <typename I>
	constexpr void reverse_stable(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		if (reverse_stable_initial_pass(begin, end))
			return;

		I i = begin + 1;
		while (begin != end)
		{
			while (i != end && *begin == *i)
				++i;
			reverse(begin, i);
			begin = i;
			i = begin + 1;
		}
	}

	template <typename I>
	constexpr void rotate(I begin, I end, I new_begin)
	{
		MONITOR_SORT_INVARIANT(begin <= end);
		MONITOR_SORT_INVARIANT(begin <= end);

		auto left_size = std::distance(begin, new_begin);
		auto right_size = std::distance(new_begin, end);

		while (left_size != 0 && right_size != 0)
		{
			if (left_size <= right_size)
			{
				swap_range_forward(begin, begin + left_size, begin + left_size);
				begin += left_size;
				right_size -= left_size;
			}
			else
			{
				swap_range_forward(begin + (left_size - right_size), begin + left_size, begin + left_size);
				left_size -= right_size;
			}
		}
	}

	template <size_t Threshold, typename I, typename Predicate>
	constexpr void search_exponential_forward(I& begin, I end, I target, Predicate predicate)
	{
		using S = difference_type_of<I>;

		const S size = unsigned_distance(begin, end);

		S previous_bound = 0;
		S bound = 1;

		while (bound < size && predicate(begin + bound, target))
		{
			previous_bound = bound;
			bound *= 2;
		}

		if (bound > size)
			bound = size;

		begin += previous_bound;
	}

	template <size_t Threshold, typename I, typename Predicate>
	constexpr void search_exponential_backward(I begin, I& end, I target, Predicate predicate)
	{
		using S = difference_type_of<I>;

		const S size = unsigned_distance(begin, end);

		S previous_bound = 0;
		S bound = 1;

		while (bound < size && predicate(end - bound, target))
		{
			previous_bound = bound;
			bound *= 2;
		}

		if (bound > size)
			bound = size;

		end -= previous_bound;
	}

	template <size_t Threshold, typename I, typename Predicate>
	constexpr void search_binary(I& begin, I& end, I target, Predicate predicate)
	{
		while (true)
		{
			const auto delta = unsigned_distance(begin, end);
			if (delta <= Threshold)
				break;
			const I p = begin + delta / 2;
			if (!predicate(p, target))
				end = p;
			else
				begin = p + 1;
		}
	}

	template <typename I, typename Predicate>
	constexpr I search_linear_forward(I begin, I end, I target, Predicate predicate)
	{
		while (begin < end && predicate(begin, target))
			++begin;
		return begin;
	}

	template <typename I, typename Predicate>
	constexpr I search_linear_backward(I begin, I end, I target, Predicate predicate)
	{
		do
		{
			--end;
		} while (end > begin && predicate(begin, target));
		return end;
	}

	template <typename I, typename Predicate>
	constexpr I hybrid_search_forward(I begin, I end, I target, Predicate predicate)
	{
		using traits_type = sort_traits<value_type_of<I>>;
		constexpr auto binary_threshold = traits_type::binary_search_threshold;
		constexpr auto linear_threshold = traits_type::linear_search_threshold;

		//search_exponential_forward<binary_threshold>(begin, end, target, predicate);
		//if (begin == end)
		//	return begin;
		search_binary<linear_threshold>(begin, end, target, predicate);
		if (begin == end)
			return begin;
		return search_linear_forward(begin, end, target, predicate);
	}

	template <typename I, typename Predicate>
	constexpr I hybrid_search_backward(I begin, I end, I target, Predicate predicate)
	{
		using traits_type = sort_traits<value_type_of<I>>;
		constexpr auto binary_threshold = traits_type::binary_search_threshold;
		constexpr auto linear_threshold = traits_type::linear_search_threshold;

		//search_exponential_backward<binary_threshold>(begin, end, target, predicate);
		//if (begin == end)
		//	return begin;
		search_binary<linear_threshold>(begin, end, target, predicate);
		if (begin == end)
			return begin;
		return search_linear_backward(begin, end, target, predicate);
	}

	template <bool Forward = true, typename I>
	constexpr I lower_bound(I begin, I end, I target)
	{
		MONITOR_SORT_INVARIANT(begin <= end);
		
		if constexpr (Forward)
			return hybrid_search_forward(begin, end, target, [](I left, I right) { return *left < *right; });
		else
			return hybrid_search_backward(begin, end, target, [](I left, I right) { return *left >= *right; });
	}

	template <bool Forward = true, typename I>
	constexpr I upper_bound(I begin, I end, I target)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		if constexpr (Forward)
			return hybrid_search_forward(begin, end, target, [](I left, I right) { return *left <= *right; });
		else
			return hybrid_search_backward(begin, end, target, [](I left, I right) { return *left > *right; });
	}

	template <typename I>
	constexpr void insertion_sort_classic(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		for (I i = begin + 1; i < end; ++i)
		{
			auto tmp = std::move(*i);
			I j = i - 1;
			for (; j >= begin && *j > tmp; --j)
				indirect_move(j + 1, j);
			indirect_move(j + 1, tmp);
		}
	}

	template <typename I>
	constexpr void insertion_sort_backwards(I begin, I end)
	{
		for (I i = end - 2; i >= begin; --i)
		{
			auto tmp = std::move(*i);
			for (I j = i; j < end - 1 && *(j - 1) <= tmp; ++j)
				indirect_move(j, j + 1);
			indirect_move(i, tmp);
		}
	}

	template <typename I>
	constexpr void unguarded_insert(I from)
	{
		auto tmp = std::move(*from);
		do
		{
			--from;
			indirect_move(from + 1, from);
		} while (*from > tmp);
		indirect_move(from + 1, tmp);
	}

	template <typename I>
	constexpr void sink_min(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		I min = begin;
		for (I i = min + 1; i < end; ++i)
			if (*i < *min)
				min = i;
		auto tmp = std::move(*min);
		for (I i = min; i > begin; --i)
			indirect_move(i, i - 1);
		indirect_move(begin, tmp);
	}

	template <typename I>
	constexpr void linear_insertion_sort_stable(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin < end);

		sink_min(begin, end);
		while (true)
		{
			++begin;
			if (begin == end)
				break;
			unguarded_insert(begin);
		}
	}

	template <typename I>
	constexpr void linear_insertion_sort_unstable(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin < end);
		for (I i = begin + 1; i < end; ++i)
		{
			if (indirect_cmplt(i, begin))
				std::iter_swap(i, begin);
			unguarded_insert(i);
		}
	}

	template <typename I>
	constexpr void hybrid_insertion_sort_stable(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin < end);

		using T = value_type_of<I>;
		using traits_type = sort_traits<T>;

		const auto size = unsigned_distance(begin, end);

		if (size < traits_type::classic_insertion_sort_threshold)
		{
			insertion_sort_classic(begin, end);
			return;
		}

		if constexpr (std::is_trivially_move_constructible<T>::value && sizeof(T) <= platform::cache_line_size)
		{
			linear_insertion_sort_stable(begin, end);
		}
		else
		{
			for (I i = begin + 1; i != end; ++i)
			{
				const I target = lower_bound(begin, end, i);
				insert_backward(target, i);
			}
		}
	}

	template <typename I>
	constexpr void hybrid_insertion_sort_unstable(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin < end);

		using T = value_type_of<I>;
		using traits_type = sort_traits<T>;

		const auto size = unsigned_distance(begin, end);

		if (size < traits_type::classic_insertion_sort_threshold)
		{
			insertion_sort_classic(begin, end);
			return;
		}

		if constexpr (std::is_trivially_move_constructible<T>::value && sizeof(T) <= platform::cache_line_size)
		{
			linear_insertion_sort_unstable(begin, end);
		}
		else
		{
			for (I i = begin + 1; i != end; ++i)
			{
				const I target = lower_bound(begin, end, i);
				insert_backward(target, i);
			}
		}
	}

	template <typename I>
	constexpr I find_reverse_end(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		do
		{
			++begin;
		} while (begin < end && *(begin - 1) >= *begin);
		return begin;
	}
	
	template <typename I>
	constexpr I find_insertion_slot(I begin, I end)
	{
		MONITOR_SORT_INVARIANT(begin <= end);

		const I target = end;
		do
		{
			--end;
		} while (end >= begin && *end > *target);
		return end + 1;
	}

	template <typename I>
	constexpr void insert_forward(I from, I to)
	{
		MONITOR_SORT_INVARIANT(to >= from);
		auto tmp = std::move(*from);
		++from;
		for (; from <= to; ++from)
			indirect_move(from - 1, from);
		indirect_move(to, tmp);
	}

	template <typename I>
	constexpr void insert_backward(I from, I to)
	{
		MONITOR_SORT_INVARIANT(to <= from);
		auto tmp = std::move(*from);
		--from;
		for (; from >= to; --from)
			indirect_move(from + 1, from);
		indirect_move(to, tmp);
	}

	template <typename K, typename I>
	constexpr void fallback_sort(I begin, I unique_begin, I end)
	{
		/*
			Invariants:
				- [unique_begin, end) is ascending.
				- [unique_begin, end) contains unique elements.
				- |end - unique_begin| is less than 2 * sqrt(|end - begin|).
		*/
	}

	template <typename I, typename K>
	constexpr void encode_size_forward(I begin, K size)
	{
		/*
			left <= right => 0
			left > right => 1
		*/

		for (K value = size; value != 0; value >>= 1)
		{
			const bool flag = (value & 1) != 0;
			I left = begin;
			++begin;
			I right = begin;
			++begin;

			if (!flag)
				continue;

			if (*left != *right)
			{
				std::iter_swap(left, right);
			}
			else
			{
				I new_right = right;
				do { ++new_right; } while (*left == *new_right);
				insert_backward(right, new_right);
			}
		}
	}

	template <typename I, typename K>
	constexpr void encode_size_backward(I end, K size)
	{
		/*
			left <= right => 0
			left > right => 1
		*/

		for (K value = size; value != 0; value >>= 1)
		{
			const bool flag = (value & 1) != 0;
			--end;
			I right = end;
			--end;
			I left = end;

			if (!flag)
				continue;

			if (*left != *right)
			{
				std::iter_swap(left, right);
			}
			else
			{
				I new_left = left;
				do { --new_left; } while (*right == *new_left);
				insert_backward(left, new_left);
			}
		}
	}

	template <typename K, typename I>
	constexpr K decode_size_forward(I begin, uint_fast8_t min_run_size)
	{
		K r = 0;
		K mask = 1;
		const I end = begin + (K)min_run_size;
		for (K mask = 1; begin + 2 <= end; mask <<= 1)
		{
			I left = begin;
			++begin;
			I right = begin;
			++begin;
			if (indirect_cmpgt(left, right))
				r |= mask;
		}
		return r;
	}

	template <typename K, typename I>
	constexpr K decode_size_backward(I end, uint_fast8_t min_run_size)
	{
		K r = 0;
		const I begin = end - (K)min_run_size;

		for (K mask = 1; end - 2 >= begin; mask <<= 1)
		{
			--end;
			I right = end;
			--end;
			I left = end;

			if (*left > *right)
				r |= mask;
		}
		return r;
	}

	template <typename I>
	constexpr I build_run(I begin, I end, uint_fast8_t min_run_size)
	{
		using traits_type = traits_type_of<I>;

		I min_end = begin + min_run_size;
		if (min_end > end)
			min_end = end;

		if (unsigned_distance(min_end, end) < min_run_size)
		{
			hybrid_insertion_sort_stable(begin, end);
			return end;
		}

		if (*begin > *(begin + 1))
		{
			const I run_end = find_reverse_end(begin, end);
			if (unsigned_distance(begin, run_end) >= min_run_size)
			{
				reverse_stable(begin, run_end);
				return run_end;
			}
		}

		for (I i = begin + 1; i < end; ++i)
		{
			const I previous = i - 1;
			if (*previous < *i)
				continue;
			if (i >= min_end)
				return i;
			const I to = find_insertion_slot(begin, i);
			if (unsigned_distance(to, i) >= min_run_size)
				return i;
			insert_backward(i, to);
		}
		return end;
	}

	/// <summary>
	/// Build runs until the end of the array. Run size is stored in place by using comparisson results as the bits of the value.
	/// </summary>
	template <typename I, typename K>
	constexpr I build_remaining_runs(I begin, I end, uint_fast8_t min_run_size, K& run_count)
	{
		while (true)
		{
			const I run_end = build_run(begin, end, min_run_size);
			const K run_size = unsigned_distance<K>(begin, run_end);

			++run_count;

			// Last run does not have its size encoded (to simplify handling of smaller runs).

			encode_size_backward(run_end, run_size);

#ifdef MONITOR_SORT_DEBUG
			const auto value = decode_size_backward<K>(run_end, min_run_size);
			MONITOR_SORT_INVARIANT(value == run_size);
#endif

			if (run_end == end)
				return begin;

			begin = run_end;
		}
	}

	template <typename I, typename K>
	constexpr K gather_unique_backward(I begin, I end, K expected_unique_count)
	{
		const I desired_found_begin = end - expected_unique_count;
		I found_begin = end - 1;
		for (I i = found_begin - 1; i >= begin && found_begin != desired_found_begin; --i)
		{
			const I target = lower_bound(found_begin, end, i) - 1;
			if (*target != *i)
			{
				insert_forward(i, target);
				--found_begin;
			}
		}
		return unsigned_distance<K>(found_begin, end);
	}

	template <typename I>
	constexpr void internal_merge_forward(I begin, I middle, I end, I buffer_begin)
	{
		MONITOR_SORT_INVARIANT(buffer_begin < begin);
		MONITOR_SORT_INVARIANT(begin < middle);
		MONITOR_SORT_INVARIANT(middle < end);

		I left = begin;
		I right = middle;
		I buffer = buffer_begin;

		while (right < end)
		{
			if (left == middle || indirect_cmpgt(left, right))
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
			swap_range_forward(buffer, buffer + unsigned_distance(middle, left), left);
	}

	template <typename I>
	constexpr void internal_merge_backward(I begin, I middle, I end, I buffer_end)
	{
		MONITOR_SORT_INVARIANT(begin < middle);
		MONITOR_SORT_INVARIANT(middle < end);
		MONITOR_SORT_INVARIANT(end < buffer_end);

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

		if (right != buffer)
		{
			while (right >= middle)
			{
				std::iter_swap(buffer, right);
				--buffer;
				--right;
			}
		}
	}

	template <typename I>
	constexpr void distribute_blocks_forward(I begin, I middle, I end, I control_begin)
	{

	}

	template <typename I, typename K>
	constexpr void merge_blocks_forward(I begin, I middle, I end, K size_sqrt)
	{
	}

	template <typename I>
	constexpr void distribute_blocks_backward(I begin, I middle, I end, I control_begin)
	{
	}

	template <typename I, typename K>
	constexpr void merge_blocks_backward(I begin, I middle, I end, K size_sqrt)
	{
	}

	template <typename I>
	constexpr void insert_internal_buffer(I begin, I internal_buffer_begin, I end)
	{
	}

	template <typename I, typename K>
	constexpr K merge_pass_forward(I begin, I end, K internal_buffer_size, I control_buffer_begin, K run_count, uint_fast8_t min_run_size)
	{
		return run_count;
	}

	template <typename I, typename K>
	constexpr K merge_pass_backward(I begin, I end, K internal_buffer_size, I control_buffer_begin, K run_count, uint_fast8_t min_run_size)
	{
		K done_run_index = run_count;
		K run_index = done_run_index;
		I buffer_begin = end;
		I buffer_end = control_buffer_begin;
		while (end > begin)
		{
			const I merge_end = end;
			
			const K right_size = decode_size_backward<K>(merge_end, min_run_size);
			MONITOR_SORT_INVARIANT(right_size != 0);

			if (right_size > internal_buffer_size)
			{
				rotate(end, buffer_end, buffer_begin);
				buffer_begin -= right_size;
				buffer_end -= right_size;
				continue;
			}

			const I merge_middle = end;
			
			const K left_size = decode_size_backward<K>(merge_middle, min_run_size);
			MONITOR_SORT_INVARIANT(left_size != 0);

			const K merged_size = left_size + right_size;
			end -= left_size;
			const I merge_begin = end;

			if (left_size > internal_buffer_size)
			{
				rotate(end, buffer_end, buffer_begin);
				buffer_begin -= merged_size;
				buffer_end -= merged_size;
				continue;
			}

			--run_index;
			hybrid_insertion_sort_stable(merge_begin, merge_middle);
			hybrid_insertion_sort_stable(merge_middle, merge_end);
			internal_merge_backward(merge_begin, merge_middle, merge_end, buffer_end);
			buffer_begin -= merged_size;
			buffer_end -= merged_size;
			encode_size_forward(buffer_end, left_size + right_size);

			if (end == begin)
				break;
		}

		if (run_index == 0)
		{
			rotate(begin, buffer_end, buffer_begin);
		}

		return (run_count / 2) + (run_count & (K)1);
	}

	template <typename I, typename K>
	constexpr void merge_all_runs(I begin, I end, K internal_buffer_size, I control_buffer_begin, K run_count, uint_fast8_t min_run_size)
	{
		while (true)
		{
			run_count = merge_pass_backward(begin, end, internal_buffer_size, control_buffer_begin, run_count, min_run_size);
			if (run_count == 1)
				break;
			run_count = merge_pass_forward(begin, end, internal_buffer_size, control_buffer_begin, run_count, min_run_size);
			if (run_count == 1)
				break;
		}
	}

	template <typename K, typename I>
	constexpr void entry_point_size_typed(I begin, I end, K size, uint_fast8_t min_run_size, I first_run_end)
	{
		using traits_type = traits_type_of<I>;

		constexpr size_t max_external_run_sizes = platform::page_size_hint / sizeof(K);

		// Build internal buffer:

		const K size_sqrt = sqrt_of(size);
		const K internal_buffer_size = size_sqrt * 2;
		const K found_unique_count = gather_unique_backward(begin, end, internal_buffer_size);

		if (found_unique_count < internal_buffer_size)
		{
			fallback_sort<K>(begin, end - found_unique_count, end);
			return;
		}

		const I control_buffer_begin = end - size_sqrt;
		const I internal_buffer_begin = end - internal_buffer_size;

		// Successfully found required unique element count, build remaining runs:

		K run_count = 1;
		build_remaining_runs(first_run_end, internal_buffer_begin, min_run_size, run_count);

		merge_all_runs(begin, internal_buffer_begin, size_sqrt, control_buffer_begin, run_count, min_run_size);

		insert_internal_buffer(begin, internal_buffer_begin, end);
	}

	template <typename I>
	constexpr void entry_point(I begin, I end)
	{
		using traits_type = traits_type_of<I>;

		const auto size = unsigned_distance(begin, end);

		if (size <= traits_type::insertion_sort_threshold)
		{
			hybrid_insertion_sort_stable(begin, end);
			return;
		}

		const uint_fast8_t size_log2 = log2_of(size);

		uint_fast8_t min_run_size = 2 * size_log2;
		if (traits_type::min_run_size > min_run_size)
			min_run_size = traits_type::min_run_size;

		// Build first run:

		const I first_run_end = build_run(begin, end, min_run_size);
		
		if (first_run_end == end)
			return;

		encode_size_backward(first_run_end, unsigned_distance(begin,first_run_end));

		// Find smallest uint*_t that can store |end - begin|:

		if (size > UINT16_MAX)
		{
			if (size > UINT32_MAX)
				entry_point_size_typed<uint64_t>(begin, end, (uint64_t)size, min_run_size, first_run_end);
			else
				entry_point_size_typed<uint32_t>(begin, end, (uint32_t)size, min_run_size, first_run_end);
		}
		else
		{
			if (size > UINT8_MAX)
				entry_point_size_typed<uint16_t>(begin, end, (uint16_t)size, min_run_size, first_run_end);
			else
				entry_point_size_typed<uint8_t>(begin, end, (uint8_t)size, min_run_size, first_run_end);
		}
	}

	template <typename RandomAccessIterator>
	constexpr void sort(RandomAccessIterator begin, RandomAccessIterator end)
	{
		using category = typename std::iterator_traits<RandomAccessIterator>::iterator_category;
		constexpr bool is_random_access_iterator = std::is_same<category, std::random_access_iterator_tag>::value;
		static_assert(is_random_access_iterator, "winter_sort only accepts random access iterator types!");

		entry_point(begin, end);
	}
}



void block_merge_monitor_sort(main_array array)
{
	monitor_sort::sort(array.begin(), array.end());
}