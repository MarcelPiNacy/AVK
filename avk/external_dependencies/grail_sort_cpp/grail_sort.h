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
#include "internal/grail_sort_common.h"



namespace grail_sort
{
	/// <summary>
	/// Sorts the range [begin, end) stably and fully in-place.
	/// </summary>
	template <typename Iterator, typename Int = ptrdiff_t>
	constexpr void sort(Iterator begin, Iterator end) GRAILSORT_NOTHROW
	{
		const auto size = std::distance(begin, end);
		detail::entry_point<Iterator, Int, false>(begin, size, Iterator(), 0);
	}

	/// <summary>
	/// Sorts the range [begin, end) stably, using (external_buffer_begin, external_buffer_end] as a temporary buffer. Recommended sizes for this buffer are 512 or the square root of the length of range to sort. The size of this buffer can be 0.
	/// </summary>
	template <typename Iterator, typename Int = ptrdiff_t>
	constexpr void sort(Iterator begin, Iterator end, Iterator external_buffer_begin, Iterator external_buffer_end) GRAILSORT_NOTHROW
	{
		const auto size = std::distance(begin, end);
		const auto buffer_size = std::distance(external_buffer_begin, external_buffer_end);
		detail::entry_point<Iterator, Int, true>(begin, size, external_buffer_begin, buffer_size);
	}
}