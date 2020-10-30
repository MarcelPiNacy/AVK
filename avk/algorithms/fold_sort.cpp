#include "all.h"
#include "../external_dependencies/fold_sort.h"

void fold_sort(main_array& array)
{
	sorting::networks::fold_sort(array.begin(), array.end());
}