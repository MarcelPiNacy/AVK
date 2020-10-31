#pragma once
#include "../avk.h"

void selection_sort(main_array& array);
void insertion_sort(main_array& array);
void bubble_sort(main_array& array);
void odd_even_sort(main_array& array);
void std_merge_sort(main_array& array);
void std_inplace_merge_sort(main_array& array);
void std_sort_heap(main_array& array);
void std_sort(main_array& array);
void std_stable_sort(main_array& array);
void block_merge_grail_sort(main_array& array);
void block_merge_grail_sort_static(main_array& array);
void block_merge_grail_sort_cpp(main_array& array);
void block_merge_grail_sort_cpp_static(main_array& array);
void bitonic_sort(main_array& array);
void fold_sort(main_array& array);
void odd_even_merge_sort(main_array& array);
void counting_sort(main_array& array);
void radix_tree_sort(main_array& array);
void msd_radix_sort(main_array& array);
void lsd_radix_sort(main_array& array);
void american_flag_sort(main_array& array);



constexpr function_ptr<void, main_array&> sort_table[] =
{
	selection_sort,
	insertion_sort,
	bubble_sort,
	odd_even_sort,
	std_merge_sort,
	std_inplace_merge_sort,
	std_sort_heap,
	std_sort,
	std_stable_sort,
	block_merge_grail_sort,
	block_merge_grail_sort_static,
	block_merge_grail_sort_cpp,
	block_merge_grail_sort_cpp_static,
	odd_even_merge_sort,
	bitonic_sort,
	fold_sort,
	counting_sort,
	radix_tree_sort,
	msd_radix_sort,
	lsd_radix_sort,
	american_flag_sort
};