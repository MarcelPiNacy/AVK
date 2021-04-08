#include "all.h"



void weave_sorting_network(main_array array)
{
	size_t n = 1;
	for (; n < array.size(); n *= 2);

	for (size_t i = 1; i < n; i *= 2)
		for (size_t j = 1; j <= i; j *= 2)
			for (size_t k = 0; k < n; k += n / j)
				for (size_t d = n / i / 2, m = 0, l = n / j - d; l >= n / j / 2; l -= d)
					for (size_t p = 0; p < d; p++, m++)
						compare_swap(array, k + m, k + l + p);
}