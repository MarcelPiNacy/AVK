#include "../avk/avk.h"



namespace selection_sort
{
	inline void run(main_array& array)
	{
		for (int i = 1; i < array.size(); ++i)
		{
			int min = i - 1;

			for (int j = i; j < array.size(); ++j)
				if (array[j] < array[min])
					min = j;

			if (min != i)
				swap(array[i], array[min]);
		}
	}
}