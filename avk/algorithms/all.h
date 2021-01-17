#pragma once
#include "../internal/main_array.h"
#include "sort_config.h"

// SELECTION:
void selection_sort(main_array array);
void std_sort_heap(main_array array);

// INSERTION:
void insertion_sort(main_array array);
void binary_tree_sort(main_array array);

// EXCHANGE:
void bubble_sort(main_array array);
void odd_even_sort(main_array array);

// MERGE:
void std_merge_sort(main_array array);
void std_inplace_merge_sort(main_array array);
void quad_sort(main_array array);

// DISTRIBUTION:
void counting_sort(main_array array);
void msd_radix_sort(main_array array);
void lsd_radix_sort(main_array array);
void american_flag_sort(main_array array);
void PARADIS_sort(main_array array);

// HYBRID:
void std_sort(main_array array);
void std_stable_sort(main_array array);
void wiki_sort(main_array array);
void grail_sort(main_array array);
void grail_sort_cpp(main_array array);
void sqrt_sort(main_array array);
void binary_msd_radix_sort(main_array array);
void radix_tree_sort(main_array array);
void ska_sort(main_array array);
void ska_sort_copy(main_array array);

// PARALLEL / SORTING NETWORK:
void bitonic_sort(main_array array);
void bitonic_sort_parallel(main_array array);
void fold_sort(main_array array);
void fold_sort_bottom_up(main_array array);
void fold_sort_parallel(main_array array);
void odd_even_merge_sort(main_array array);
void parallel_merge_sort(main_array array);

// "EXPERIMENTAL":
void custom_radix_sort(main_array array);
void block_merge_monitor_sort(main_array array);
void stackless_quick_sort(main_array array);
void gambit_insertion_sort(main_array array);



constexpr function_ptr<void, main_array> sort_table[] =
{
	selection_sort,
	std_sort_heap,
	
	insertion_sort,
	binary_tree_sort,
	
	bubble_sort,
	odd_even_sort,
	
	std_merge_sort,
	std_inplace_merge_sort,
	
	counting_sort,
	msd_radix_sort,
	lsd_radix_sort,
	american_flag_sort,
	
	
	std_sort,
	std_stable_sort,
	wiki_sort,
	grail_sort,
	grail_sort_cpp,
	sqrt_sort,
	binary_msd_radix_sort,
	radix_tree_sort,
	ska_sort,
	ska_sort_copy,

	bitonic_sort,
	fold_sort,
	fold_sort_bottom_up,
	odd_even_merge_sort,

	custom_radix_sort,
	block_merge_monitor_sort,
	gambit_insertion_sort,
	stackless_quick_sort,
};