#pragma once
#include <intrin.h>
#include <new>



namespace platform
{
	inline void yield_cpu() noexcept
	{
#if defined(_M_AMD64) || defined(_M_IX86)
		_mm_pause();
#else
		__nop();
#endif
	}

	void yield_thread() noexcept;

	constexpr auto cache_line_size = std::hardware_destructive_interference_size;
}