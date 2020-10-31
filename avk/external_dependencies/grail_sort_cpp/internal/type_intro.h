#pragma once
#include <cstdint>
#include <type_traits>
#include "grail_sort_platform.h"



namespace grail_sort::detail::introspection
{
	enum : uint32_t
	{
		binary_search_threshold_fundamental_type_multiplier = 2,
	};

	template <typename T, typename K = ptrdiff_t>
	constexpr K locality_aware_fallback_threshold() noexcept
	{
		constexpr K base = (K)(platform::cache_line_size / sizeof(T));
		return base * (K)binary_search_threshold_fundamental_type_multiplier;
	}

	template <typename T, typename K = ptrdiff_t>
	constexpr K exponential_search_fallback_threshold() noexcept
	{
		return (K)(platform::page_size / sizeof(T));
	}

	template <typename T, typename K = ptrdiff_t>
	constexpr K interpolation_search_fallback_threshold() noexcept
	{
		static_assert(std::is_arithmetic_v<T>);
		return locality_aware_fallback_threshold<T, K>();
	}

	template <typename T, typename K = ptrdiff_t>
	constexpr K binary_search_fallback_threshold() noexcept
	{
		if constexpr (std::is_fundamental_v<T>)
		{
			return locality_aware_fallback_threshold<T, K>();
		}
		else
		{
			if constexpr (std::is_trivially_move_constructible_v<T>)
			{
				return (K)32;
			}
			else
			{
				return (K)8;
			}
		}
	}
}