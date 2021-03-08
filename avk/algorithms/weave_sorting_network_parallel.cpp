#include "all.h"
#include <vector>
#include <thread>

using std::vector;
using std::thread;

static void circle(main_array array, uint pos, uint len, uint gap)
{
	if (len < 2)
		return;

	parallel_for<uint>(0, (len - 1) / 2, [=](uint i)
	{
		i *= gap;
		compare_swap(array, pos + i, pos + (len - 1) * gap - i);
	});

	circle(array, pos, len / 2, gap);
	if (pos + len * gap / 2 < array.size())
		circle(array, pos + len * gap / 2, len / 2, gap);
}

static void weave_sorting_network_parallel_kernel(main_array array, uint pos, uint size, uint gap)
{
	if (size < 2)
		return;

	if (size < 128)
	{
		weave_sorting_network_parallel_kernel(array, pos, size / 2, 2 * gap);
		weave_sorting_network_parallel_kernel(array, pos + gap, size / 2, 2 * gap);
	}
	else
	{
		parallel_for(0, 2, [=](int i)
		{
			if (i == 0)
				weave_sorting_network_parallel_kernel(array, pos, size / 2, 2 * gap);
			else
				weave_sorting_network_parallel_kernel(array, pos + gap, size / 2, 2 * gap);
		});
	}

	circle(array, pos, size, gap);
}

void weave_sorting_network_parallel(main_array array)
{
	uint n = 1;
	for (; n < array.size(); n *= 2);

	run_as_parallel([=]() { weave_sorting_network_parallel_kernel(array, 0, n, 1); });
}