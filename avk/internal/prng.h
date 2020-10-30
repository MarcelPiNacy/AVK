#pragma once
#undef min
#undef max
#include <cstdint>



void		romu_duo_set_seed(uint64_t seed) noexcept;
uint64_t	romu_duo_get() noexcept;



struct romu_duo_functor
{
	using result_type = uint64_t;

	static constexpr auto min() { return 0; }
	static constexpr auto max() { return UINT64_MAX; }

	inline auto operator()() noexcept
	{
		return romu_duo_get();
	}
};