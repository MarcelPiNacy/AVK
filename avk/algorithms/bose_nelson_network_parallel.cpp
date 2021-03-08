#include "all.h"
#include <thread>

using std::thread;

static void bose_nelson_merge(main_array array, uint start1, uint len1, uint start2, uint len2)
{
    if (len1 == 1 && len2 == 1)
    {
        compare_swap(array, start1, start2);
    }
    else if (len1 == 1 && len2 == 2)
    {
        compare_swap(array, start1, start2 + 1);
        compare_swap(array, start1, start2);
    }
    else if (len1 == 2 && len2 == 1)
    {
        compare_swap(array, start1, start2);
        compare_swap(array, start1 + 1, start2);
    }
    else
    {
        uint mid1 = len1 / 2;
        uint mid2 = len1 % 2 == 1 ? len2 / 2 : (len2 + 1) / 2;
        thread threads[3] = {};
        threads[0] = thread(bose_nelson_merge, array, start1, mid1, start2, mid2);
        threads[1] = thread(bose_nelson_merge, array, start1 + mid1, len1 - mid1, start2 + mid2, len2 - mid2);
        threads[2] = thread(bose_nelson_merge, array, start1 + mid1, len1 - mid1, start2, mid2);
        for (thread& t : threads)
            t.join();
    }
}

static void bose_nelson_core(main_array array, uint start, uint length)
{
    if (length < 2)
        return;
    uint mid = length / 2;
    thread threads[2] = {};
    threads[0] = thread(bose_nelson_core, array, start, mid);
    threads[1] = thread(bose_nelson_core, array, start + mid, length - mid);
    for (thread& t : threads)
        t.join();
    bose_nelson_merge(array, start, mid, start + mid, length - mid);
}

void bose_nelson_network_parallel(main_array array)
{
    run_as_parallel([=]() { bose_nelson_core(array, 0, array.size()); });
}