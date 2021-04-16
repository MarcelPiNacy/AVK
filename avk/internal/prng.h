#pragma once
#undef min
#undef max
#include <cstdint>



struct romu2jr
{
	uint64_t x, y;

	void set_seed(uint64_t seed);
	uint64_t get();
};