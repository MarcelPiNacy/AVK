#pragma once
#include <cstdint>
#include "main_array.h"

using sort_function_pointer = void(*)(main_array main_array);

namespace algorithm_thread
{
	void assign_body(sort_function_pointer sort);
	void launch();
	bool is_paused();
	bool is_idle();
	void pause();
	void resume();
	void abort_sort();
	void signal();
	void await(uint32_t timeout_ms = UINT32_MAX);
	void terminate();
}