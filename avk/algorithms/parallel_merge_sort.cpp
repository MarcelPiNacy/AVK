#include "all.h"
#include <algorithm>
#include <thread>
#include <vector>

void merge(item* begin, item* middle, item* end)
{
	std::inplace_merge(begin, middle, end);
}

void parallel_merge_sort(main_array array)
{
	run_as_parallel([=]()
	{
		using std::vector;
		using std::thread;

		for (uint run_size = 1; run_size < array.size(); run_size *= 2)
		{
			vector<thread> threads;
			threads.reserve(array.size() / run_size);
			item* begin = array.begin();
			while (true)
			{
				item* middle = begin + run_size;
				if (middle >= array.end())
					break;
				item* end = middle + run_size;
				if (end > array.end())
					end = array.end();
				threads.push_back(thread(merge, begin, middle, end));
				begin = end;
			}

			for (thread& t : threads)
				t.join();
		}
	});
}