#include "all.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <iterator>



template <typename I, typename F>
bool fast_swap(I begin, I end, F&& is_greater)
{
	using std::move;
	using std::distance;
	using std::iter_swap;

	const auto size = distance(begin, end);
	const bool even_size = (size & 1) == 0;

	while (begin <= end)
	{
		if (!is_greater(begin, begin + 1))
		{
			begin += 2;
			continue;
		}

		I start = begin;
		I previous = I();

	Swapper:

		begin += 2;
		if (begin <= end)
		{
			if (is_greater(begin, begin + 1))
			{
				if (is_greater(begin - 1, begin))
					goto Swapper;
				iter_swap(begin, begin + 1);
			}
			previous = begin - 1;
			begin += 2;
		}
		else
		{
			if (start == begin)
			{
				if (even_size || is_greater(begin - 1, begin))
				{
					previous = start + size - 1;
					while (start < previous)
					{
						auto tmp = move(*start);
						*start = move(*previous);
						++start;
						*previous = move(tmp);
						--previous;
					}
					return true;
				}
				previous = begin - 1;
			}
		}

		while (start < previous)
		{
			auto tmp = move(*start);
			assert(previous != I());
			*start = move(*previous);
			++start;
			*previous = move(tmp);
			--previous;
		}
	}

	return false;
}

void quad_sort(main_array array)
{
	fast_swap(array.begin(), array.end(), [](const item* lhs, const item* rhs) { return *lhs > *rhs; });
}