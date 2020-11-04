#include "all.h"
#include <set>



void binary_tree_sort(main_array& array)
{
	std::multiset<item> tree;
	for (item& e : array)
		tree.insert(e);
	uint offset = 0;
	for (const item& e : tree)
	{
		array[offset] = e;
		++offset;
	}
}
