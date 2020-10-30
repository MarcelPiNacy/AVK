#pragma once
#undef min
#undef max
#include <cstdint>



void		romu_duo_set_seed(uint64_t seed) noexcept;
uint64_t	romu_duo_get() noexcept;