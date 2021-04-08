#include "all.h"
#include "sort_config.h"

static int gs_cmp(item* l, item* r)
{
	return (int)compare(*l, *r);
}
#define SORT_CMP gs_cmp
#define SORT_TYPE item
#include "../external_dependencies/GrailSort.h"
#include <vector>

void grail_sort(main_array array)
{
	if (sort_config::grail_sort_buffer_size != 0)
	{
		std::vector<item> buffer;
		buffer.resize(sort_config::grail_sort_buffer_size);
		grail_commonSort(array.data(), (int)array.size(), buffer.data(), (int)buffer.size());
	}
	else
	{
		GrailSort(array.data(), (int)array.size());
	}
}
