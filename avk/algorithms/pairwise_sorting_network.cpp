#include "all.h"



void pairwise_sorting_network(main_array array)
{
    uint length = array.size();
    uint a = 1;
    uint b = 0;
    uint c = 0;
    uint d = 0;
    uint e = 0;
    while (a < length)
    {
        b = a;
        c = 0;
        while (b < length)
        {
            if (array[b - a] > array[b])
            {
                swap(array, b - a, b);
            }
            c = (c + 1) % a;
            b++;
            if (c == 0)
            {
                b += a;
            }
        }
        a *= 2;
    }
    a /= 4;
    e = 1;
    while (a > 0)
    {
        d = e;
        while (d > 0)
        {
            b = ((d + 1) * a);
            c = 0;
            while (b < length)
            {
                if (array[b - (d * a)] > array[b])
                {
                    swap(array, b - (d * a), b);
                }
                c = (c + 1) % a;
                b++;
                if (c == 0)
                {
                    b += a;
                }
            }
            d /= 2;
        }
        a /= 2;
        e = (e * 2) + 1;
    }
}