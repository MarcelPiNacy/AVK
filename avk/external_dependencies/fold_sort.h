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

#pragma once
#include <cstdint>
#include <algorithm>

namespace sorting::networks
{
	namespace detail
	{
		template <typename Iterator>
		constexpr void halver(Iterator low, Iterator high)
		{
			while (low < high)
			{
				if (*high < *low)
					std::swap(*high, *low);
				++low;
				--high;
			}
		}
	}

	/// <summary>
	/// Sorts the range [begin, end). WARNING: Assumes (end - begin) is a power of 2.
	/// Comparissons: O(N * log^2(N)) best case, O(N * log^2(N)) worst case.
	/// Swaps: O(1) best case (0 swaps), O(N * log^2(N)) worst case.
	/// </summary>
	/// <typeparam name="Iterator">A bidirectional iterator type.</typeparam>
	/// <param name="begin">An iterator to the start of the collection.</param>
	/// <param name="end">An iterator to the end of the collection.</param>
	template <typename Iterator>
	constexpr void fold_sort(Iterator begin, Iterator end)
	{
		const auto size = std::distance(begin, end);
		for (auto i = size / 2; i > 0; i /= 2)
		{
			for (auto j = size; j >= i; j /= 2)
			{
				for (auto low = begin; low < end;)
				{
					auto high = low + j;
					detail::halver(low, high - 1);
					low = high;
				}
			}
		}
	}
}