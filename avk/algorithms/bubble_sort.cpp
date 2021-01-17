#include "all.h"
#include <iterator>

template <typename I>
constexpr void bubble_sort(I begin, I end)
{
	while (true)
	{
		bool loop = false;
		for (I i = begin + 1; i != end; ++i)
		{
			bool flag = *(i - 1) > *i;
			if (flag)
				std::iter_swap(i - 1, i);
			if (flag)
				loop = true;
		}
		if (!loop)
			break;
	}
}

void bubble_sort(main_array array)
{
	bubble_sort(array.begin(), array.end());
}