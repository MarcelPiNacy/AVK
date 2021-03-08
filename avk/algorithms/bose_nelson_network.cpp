#include "all.h"


static void rangeComp(main_array array, uint a, uint b, uint offset)
{
	int half = (b - a) / 2, m = a + half;
	a += offset;

	for (uint i = 0; i < half - offset; i++)
		if ((i & ~offset) == i && m + i < array.size())
			compare_swap(array, a + i, m + i);
}

void bose_nelson_network(main_array array)
{
	uint current_size = 1 << (uint)(ceilf(logf((float)array.size()) / logf(2))); //@TODO AAAAH
	for (uint k = 2; k <= current_size; k *= 2)
		for (uint j = 0; j < k / 2; j++)
			for (uint i = 0; i + j < array.size(); i += k)
				rangeComp(array, i, i + k, j);
}