#include "all.h"
#include <set>



void binary_tree_sort(main_array& array)
{
	std::multiset<item> tree;
	for (item& e : array)
	{
		scoped_highlight hl(e.flags);
		tree.insert(e);
	}
	uint offset = 0;
	for (const item& e : tree)
	{
		scoped_highlight hl(array[offset].flags);
		array[offset] = e;
		++offset;
	}
}
