#include "all.h"

static int ss_cmp(item* l, item* r)
{
	return (int)compare(*l, *r);
}
#define SORT_CMP ss_cmp
#define SORT_TYPE item
#include "../external_dependencies/SqrtSort/SqrtSort.h"

void sqrt_sort(main_array array)
{
	SqrtSort(array.data(), (int)array.size());
}