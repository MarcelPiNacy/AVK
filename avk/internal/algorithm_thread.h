#pragma once
#include <cstdint>
#include "main_array.h"

using sort_function_pointer = void(*)(main_array);

namespace algorithm_thread
{
	void launch(sort_function_pointer sort);
	bool is_paused();
	bool is_idle();
	void pause();
	void resume();
	void terminate();
}