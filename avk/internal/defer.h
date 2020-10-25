#pragma once
#include <utility>



template <typename F>
struct defer_state
{
	F callback;

	constexpr defer_state(F&& function) noexcept
		: callback(std::forward<F>(function))
	{
	}

	~defer_state()
	{
		callback();
	}
};



#define DEFER_CONCAT_IMPL(L, R) L##R
#define DEFER_CONCAT(L, R) DEFER_CONCAT_IMPL(L, R)
#define DEFER defer_state DEFER_CONCAT(defer_, __COUNTER__) = [&]()