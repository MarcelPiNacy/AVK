#include "all.h"
#include "../external_dependencies/ska_sort/ska_sort.hpp"
#include <vector>

void ska_sort_copy(main_array array)
{
	auto buffer = std::vector<item>(array.size());
	ska_sort_copy(array.begin(), array.end(), buffer.begin(), [](const item& e) { return e.value; });
}