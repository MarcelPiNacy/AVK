#include "all.h"
#include <vector>
#include <thread>

using std::vector;
using std::thread;

static void range_halver(main_array array, uint low, uint high, uint count)
{
	uint k = 0;
	while (low < high && k != count)
	{
		compare_swap(array, low, high);
		++low;
		--high;
		++k;
	}
}

static void halver(main_array array, uint low, uint high)
{
	vector<thread> threads;

	while (low < high)
	{
		uint n = 0;
		uint l = low;
		uint h = high;
		while (low < high && n < 64)
		{
			++n;
			++low;
			--high;
		}
		threads.push_back(thread(range_halver, array, l, h, n));
	}

	for (thread& t : threads)
		t.join();
}

void fold_sort_parallel(main_array array)
{
	uint size = array.size();
	for (uint i = size / 2; i > 0; i /= 2)
	{
		for (uint j = size; j >= i; j /= 2)
		{
			vector<thread> threads;
			threads.reserve(size / j);

			for (uint i = 0; i < size; i += j)
				threads.push_back(thread(halver, array, i, i + j - 1));

			for (thread& t : threads)
				t.join();
		}
	}
}