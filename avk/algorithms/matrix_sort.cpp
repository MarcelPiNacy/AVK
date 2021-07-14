#include "all.h"
#include <algorithm>

template <typename I>
struct MatrixSorter
{
    static uint8_t CountLeadingZeros(size_t value)
    {
        if (value == 0)
            return 1;
        return (uint8_t)__lzcnt64(value);
    }

    static uint32_t Sqrt(size_t value)
    {
        return (uint32_t)round(sqrt(value));
    }

    static constexpr size_t FallbackThreshold = 16;
    static constexpr size_t MaxDepth = 50;

    static void SortCore(I begin, size_t count, size_t step)
    {
    }
};

template <typename I>
static void MatrixSort(I begin, I end)
{
    size_t count = std::distance(begin, end);
    MatrixSorter<item*>::SortCore(begin, count, count);
}

void matrix_sort(main_array array)
{
    MatrixSort(array.begin(), array.end());
}