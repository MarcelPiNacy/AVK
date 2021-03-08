#include "all.h"



void weave_sorting_network(main_array array)
{
	uint n = 1;
	for (; n < array.size(); n *= 2);

	for (uint i = 1; i < n; i *= 2)
		for (uint j = 1; j <= i; j *= 2)
			for (uint k = 0; k < n; k += n / j)
				for (uint d = n / i / 2, m = 0, l = n / j - d; l >= n / j / 2; l -= d)
					for (uint p = 0; p < d; p++, m++)
						compare_swap(array, k + m, k + l + p);
}