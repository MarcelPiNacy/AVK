#pragma once
#include <cstdint>
#include "main_array.h"

using sort_function_pointer = void(*)(main_array& main_array);

namespace algorithm_thread
{
	void assign_sort(sort_function_pointer sort) noexcept;
	void launch() noexcept;
	bool is_paused() noexcept;
	bool is_idle() noexcept;
	void pause() noexcept;
	void resume() noexcept;
	void abort_sort() noexcept;
	void signal() noexcept;
	void await(uint32_t timeout_ms = UINT32_MAX) noexcept;
	void terminate() noexcept;
}