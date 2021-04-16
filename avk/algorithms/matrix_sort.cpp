#include "all.h"
#include <cmath>

static void gapped_reverse(main_array array, size_t start, size_t end, size_t gap)
{
    for (size_t i = start, j = end; i < j; i += gap, j -= gap)
        swap(array, i, j - gap);
}

static void matrix_sort(main_array array, size_t start, size_t end, size_t gap, bool ascending)
{
    size_t length = (end - start) / gap;
    if (length < 2)
        return;
    if (length == 2)
    {
        if ((array[start] > array[start + gap]) && ascending)
            swap(array[start], array[start + gap]);
        return;
    }
    size_t width = (size_t)sqrt(length);
    while ((length % width) != 0)
        --width;
    for (size_t i = start + width * gap; i < end; i += 2 * width * gap)
    {
        gapped_reverse(array, i, i + width * gap, gap);
    }
    for (size_t i = start; i < end; i += width * gap)
    {
        matrix_sort(array, i, i + width * gap, gap, ascending);
        ascending = !ascending;
    }
}

void matrix_sort(main_array array)
{
    matrix_sort(array, 0, array.size(), 1, true);
}