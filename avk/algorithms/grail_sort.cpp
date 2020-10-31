#include "all.h"

static int gs_cmp(item* l, item* r)
{
	return compare(*l, *r);
}
#define SORT_CMP gs_cmp
#define SORT_TYPE item
#include "../external_dependencies/GrailSort.h"

void block_merge_grail_sort(main_array& array)
{
	GrailSort(array.begin(), array.size());
}

void block_merge_grail_sort_static(main_array& array)
{
	GrailSortWithBuffer(array.begin(), array.size());
}