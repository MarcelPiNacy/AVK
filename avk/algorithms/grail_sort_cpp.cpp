#include "all.h"
#include "../external_dependencies/grail_sort_cpp/grail_sort.h"

void block_merge_grail_sort_cpp(main_array& array)
{
	grail_sort::sort(array.begin(), array.end());
}