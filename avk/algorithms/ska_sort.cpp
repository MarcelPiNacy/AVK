#include "all.h"
#include "../external_dependencies/ska_sort/ska_sort.hpp"

void ska_sort(main_array array)
{
	ska_sort(array.begin(), array.end(), [](const item& e) { return e.value; });
}