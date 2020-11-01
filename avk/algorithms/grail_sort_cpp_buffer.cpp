#include "all.h"
#include "../external_dependencies/grail_sort_cpp/grail_sort.h"

static thread_local item buffer[512] = {};

void block_merge_grail_sort_cpp_buffered(main_array& array)
{
	grail_sort::sort(array.begin(), array.end(), std::begin(buffer), std::end(buffer));
}