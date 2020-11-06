#include "all.h"

static int gs_cmp(item* l, item* r)
{
	return compare(*l, *r);
}
#define SORT_CMP gs_cmp
#define SORT_TYPE item
#include "../external_dependencies/GrailSort.h"
#include <vector>

void block_merge_grail_sort(main_array& array)
{
	if (sort_config::grail_sort_buffer_size != 0)
	{
		std::vector<item> buffer;
		buffer.resize(sort_config::grail_sort_buffer_size);
		grail_commonSort(array.data(), array.size(), buffer.data(), buffer.size());
	}
	else
	{
		GrailSort(array.data(), array.size());
	}
}