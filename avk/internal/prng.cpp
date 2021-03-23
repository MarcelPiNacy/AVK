#include "prng.h"
#include <intrin.h>

static thread_local uint64_t rmdx, rmdy;

void romu2jr_set_seed(uint64_t seed)
{
	rmdx = seed ^ 0x9e3779b97f4a7c15;
	rmdy = seed ^ 0xd1b54a32d192ed03;
}

uint64_t romu2jr_get()
{
	const uint64_t result = rmdx;
	rmdx = 15241094284759029579 * rmdy;
	rmdy = rmdy - result;
	rmdy = _rotl64(rmdy, 27);
	return result;
}