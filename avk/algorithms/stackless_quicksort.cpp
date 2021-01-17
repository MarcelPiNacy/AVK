#include "all.h"

/*
	MIT License
	
	Copyright (c) 2020 aphitorite
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

template <typename I>
constexpr void insertion_sort_classic(I begin, I end)
{
	for (I i = begin + 1; i < end; ++i)
	{
		auto tmp = std::move(*i);
		I j = i - 1;
		for (; j >= begin && *j > tmp; --j)
			j[1] = *j;
		j[1] = tmp;
	}
}

template <typename I>
constexpr void median_of_three(I begin, I end)
{
	auto m = begin + std::distance(begin, end) / 2;

	if (*begin > * m)
		swap(*m, *(end - 1));

	if (*m > * (end - 1))
	{
		swap(*begin, *m);

		if (*begin > *m)
			return;
	}

	swap(*begin, *m);
}

template <typename I>
constexpr I partition(I begin, I end)
{
	I i = begin;
	I j = end;
	
	median_of_three(begin, end);

	while (true)
	{
		do { ++i; } while (i < end && *i < *begin);
		do { --j; } while (j >= begin && *j > *begin);

		if (i < j)
		{
			std::swap(*i, *j);
		}
		else
		{
			std::swap(*begin, *j);
			return j;
		}
	}
}

template <typename I, typename T>
constexpr T exchange(I from, T& value)
{
	T r = std::move(*from);
	*from = std::move(value);
	return r;
}

template <typename I, typename T>
constexpr I linear_search(I begin, I end, T& target)
{
	while (begin < end && *begin < target)
		++begin;
	return begin;
}

template <typename I>
constexpr void iterative_quicksort(I begin, I end)
{
	I max = begin;
	for (I i = max + 1; i < end; ++i)
		if (*i < *max)
			max = i;
	auto tmp = *max;
	I p = partition(begin, end);
	I limit = p;
	I i = begin;
	*p = *(p + 1);
	tmp = exchange(p, tmp);
	while (i < end)
	{
		while (limit - i > 8)
		{
			p = partition(i, limit);
			limit = p;
			*p = *(p + 1);
			tmp = exchange(p, tmp);
		}
		insertion_sort_classic(i, limit);

		i = limit + 1;
		tmp = exchange(limit, tmp);
		*limit = *(limit - 1);
		limit = linear_search(i, end, tmp);
	}

}

void stackless_quick_sort(main_array array)
{
	iterative_quicksort(array.begin(), array.end());
}