#include "prng.h"
#include <intrin.h>

void romu2jr::set_seed(uint64_t seed)
{
	x = seed ^ 0x9e3779b97f4a7c15;
	y = seed ^ 0xd1b54a32d192ed03;
}

uint64_t romu2jr::get()
{
	const uint64_t result = x;
	x = 15241094284759029579 * y;
	y = y - result;
	y = _rotl64(y, 27);
	return result;
}