#include "all.h"
#include "../external_dependencies/grail_sort_cpp/grail_sort.h"
#include <vector>

void block_merge_grail_sort_cpp(main_array& array)
{
	if (sort_config::grail_sort_buffer_size != 0)
	{
		std::vector<item> buffer;
		buffer.resize(sort_config::grail_sort_buffer_size);
		grail_sort::sort(array.data(), array.data() + array.size(), buffer.data(), buffer.data() + buffer.size());
	}
	else
	{
		grail_sort::sort(array.data(), array.data() + array.size());
	}
}