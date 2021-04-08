#include "all.h"


static void ranged_cas(main_array array, size_t a, size_t b, size_t offset)
{
	size_t half = (b - a) / 2, m = a + half;
	a += offset;

	for (size_t i = 0; i < half - offset; i++)
		if ((i & ~offset) == i && m + i < array.size())
			compare_swap(array, a + i, m + i);
}

void bose_nelson_network(main_array array)
{
	size_t current_size = (size_t)1 << (size_t)(ceilf(logf((float)array.size()) / logf(2))); //@TODO AAAAH
	for (size_t k = 2; k <= current_size; k *= 2)
		for (size_t j = 0; j < k / 2; j++)
			for (size_t i = 0; i + j < array.size(); i += k)
				ranged_cas(array, i, i + k, j);
}