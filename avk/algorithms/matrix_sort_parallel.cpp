#include "all.h"

#include <Comet.hpp>
#include <iterator>
#include <algorithm>
#include <bit>

namespace Detail
{
    template <typename I>
    struct MatrixSort
    {
        // Courtesy of stackoverflow.
        static constexpr size_t Sqrt(size_t n)
        {
            uint8_t shift = n == 0 ? 1 : 64 - std::countl_zero(n);
            shift += shift & 1;
            size_t result = 0;
            do
            {
                shift -= 2;
                result <<= 1;
                result |= 1;
                result ^= result * result > (n >> shift);
            } while (shift != 0);
            return result;
        }

        // Sorts a pair of values "step" elements apart. "forward" controls the direction.
        static constexpr void CompareSwap(I lhs, I rhs, I limit, bool forward)
        {
            if (rhs < limit)
            {
                I l = forward ? rhs : lhs;
                I r = forward ? lhs : rhs;
                if (*l < *r)
                    std::iter_swap(l, r);
            }
        }

        // Sorting network for up to 4 elements.
        template <size_t N>
        static constexpr void SortingNetwork(I begin, I limit, size_t step, bool forward)
        {
            static_assert(N <= 4);

            using IPair = std::pair<size_t, size_t>;

            constexpr IPair sorting_network_n3[] =
            {
                { 1, 2 },
                { 0, 1 },
                { 1, 2 }
            };

            constexpr IPair sorting_network_n4[] =
            {
                { 0, 2 }, { 1, 3 },
                { 0, 1 }, { 2, 3 },
                { 1, 2 }
            };

            switch (N)
            {
            case 0:
            case 1:
            case 2:
                return CompareSwap(begin, begin + step, limit, forward);
            case 3:
                for (auto e : sorting_network_n3)
                    CompareSwap(begin + e.first * step, begin + e.second * step, limit, forward);
                break;
            case 4:
                for (auto e : sorting_network_n4)
                    CompareSwap(begin + e.first * step, begin + e.second * step, limit, forward);
                break;
            default:
                break;
            }
        }

        // Recursively calls MatrixSortCore on each row of the virtual matrix.
        static constexpr void SortRows(I begin, I end, size_t width, size_t width_x_step, size_t step, bool forward)
        {
            auto n = std::distance(begin, end) / width_x_step;
            Comet::Counter ctr = n;
            auto options = Comet::TaskOptions::Default();
            options.counter = &ctr;

            while (begin < end)
            {
                I next = begin + width_x_step;
                if (next > end)
                    next = end;
                Comet::Dispatch([=]()
                {
                    Core(begin, next, width, step, forward);
                }, options);
                begin = next;
                forward = !forward;
            }

            ctr.Await();
        }

        // Recursively calls MatrixSortCore on each column of the virtual matrix.
        static constexpr void SortColumns(I begin, I end, size_t width, size_t width_x_step, size_t step, bool forward)
        {
            I limit = begin + width_x_step;
            auto n = std::distance(begin, limit) / step;
            Comet::Counter ctr = n;
            auto options = Comet::TaskOptions::Default();
            options.counter = &ctr;

            for (; begin < limit; begin += step)
            {
                I high = begin + width * width_x_step;
                if (high > end)
                    high = end;
                Comet::Dispatch([=]()
                {
                    Core(begin, high, width, width_x_step, forward);
                }, options);
            }

            ctr.Await();
        }

        static constexpr void ReverseOddRows(I begin, I end, size_t width, size_t width_x_step, size_t step, bool forward)
        {
            auto wxs2 = width_x_step * 2;
            auto n = std::distance(begin, end) / wxs2;
            Comet::Counter ctr = n;
            auto options = Comet::TaskOptions::Default();
            options.counter = &ctr;

            begin += width_x_step; //Skip non-reversed rows
            for (; begin < end; begin += wxs2)
            {
                I low = begin;
                I high = begin + width_x_step - step;
                if (high >= end)
                    high = end - step;
                while (low < high)
                {
                    Comet::Dispatch([=]()
                    {
                        std::iter_swap(low, high);
                    }, options);
                    low += step;
                    high -= step;
                }
            }
        }

        // MatrixSort core routine.
        static constexpr void Core(I begin, I end, size_t width, size_t step, bool forward)
        {
            switch (width)
            {
            case 0:
            case 1: return;
            case 2: return SortingNetwork<2>(begin, end, step, forward);
            case 3: return SortingNetwork<3>(begin, end, step, forward);
            case 4: return SortingNetwork<4>(begin, end, step, forward);
            default:
                break;
            }

            auto new_width = Sqrt(width);
            auto width_x_step = new_width * step;
            for (; width > 0; width >>= 2)
            {
                SortRows(begin, end, new_width, width_x_step, step, forward);
                SortColumns(begin, end, new_width, width_x_step, step, forward);
                ReverseOddRows(begin, end, new_width, width_x_step, step, forward);
            }
        }
    };
}

template <typename I>
constexpr void MatrixSortParallel(I begin, I end)
{
    Detail::MatrixSort<I>::Core(begin, end, std::distance(begin, end), 1, true);
}



void matrix_sort_parallel(main_array array)
{
    MatrixSortParallel(array.begin(), array.end());
}