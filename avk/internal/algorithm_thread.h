#pragma once
#include <cstdint>
#include "main_array.h"

using sort_function_pointer = void(*)(main_array& main_array);

namespace algorithm_thread
{
	void assign_sort(sort_function_pointer sort) noexcept;
	void initialize() noexcept;
	bool is_idle() noexcept;
	void await(uint32_t timeout_ms = UINT32_MAX) noexcept;
}