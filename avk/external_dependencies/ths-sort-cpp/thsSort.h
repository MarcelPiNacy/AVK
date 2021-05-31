/*
Median-of-Sixteen Adaptive QuickSort 2020-2021 Copyright (C) thatsOven
thatsOven's Adaptive MergeSort 2020-2021 Copyright (C) thatsOven
featureSort 2020-2021 Copyright (C) thatsOven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <math.h>
#include <vector>

template <typename T>
struct MinMaxPair {
    T min;
    T max;
};

constexpr int incs[] = {48, 21, 7, 3, 1};
constexpr int medianOfSixteenSwaps[] = {
    1,  2,  3,  4, 5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
    1,  3,  5,  7, 9, 11, 13, 15,  2,  4,  6,  8, 10, 12, 14, 16,
    1,  5,  9, 13, 2,  6, 10, 14,  3,  7, 11, 15,  4,  8, 12, 16,
    1,  9,  2, 10, 3, 11,  4, 12,  5, 13,  6, 14,  7, 15,  8, 16,
    6, 11,  7, 10, 4, 13, 14, 15,  8, 12,  2,  3,  5,  9,
    2,  5,  8, 14, 3,  9, 12, 15,  6,  7, 10, 11,
    3,  5, 12, 14, 4,  9,  8, 13,
    7,  9, 11, 13, 4,  6,  8, 10, 
    4,  5,  6,  7, 8,  9, 10, 11, 12, 13,
    7,  8,  9, 10
};
constexpr int incsLen = 5, medianOfSixteenSwapsLen = 120;

template <typename T>
int stdcompare(T a, T b) {
    return (a > b) - (b > a);
}

template <typename T>
struct thsSort {
    int (*compare)(T, T);

    thsSort(int (*comparefn)(T, T) = stdcompare<T>) {
        compare = comparefn;
    }
    ~thsSort() {}

    void swap(T* array, int a, int b) {
        T tmp    = array[a];
        array[a] = array[b];
        array[b] = tmp;
    }

    void reverse(T* array, int a, int b) {
        for (--b; a < b; a++, b--)
            swap(array, a, b);
    }

    void siftDown(T* array, int root, int dist, int start) {
        while (root <= dist / 2) {
            int leaf = 2 * root;

            if (leaf < dist && compare(array[start + leaf - 1], array[start + leaf]) < 0)
                leaf++;

            if (compare(array[start + root - 1], array[start + leaf - 1]) < 0) {
                swap(array, start + root - 1, start + leaf - 1);
                root = leaf;
            } else break;
        }
    }

    void heapify(T* array, int low, int high) {
        int length = high - low;
        
        for (int i = length / 2; i >= 1; i--)
            siftDown(array, i, length, low);
    }

    void maxHeapSort(T* array, int start, int length) {
        heapify(array, start, length);

        for (int i = length - start; i > 1; i--) {
            swap(array, start, start + i - 1);
            siftDown(array, 1, i - 1, start);
        }
    }

    void uncheckedInsertionSort(T* array, int a, int b) {
        for (int i = a + 1; i < b; i++) {
            if (compare(array[i], array[a]) < 0)
                swap(array, i, a);

            T key = array[i];

            int j = i - 1;
            for (; compare(key, array[j]) < 0; j--)
                array[j + 1] = array[j];
            array[j + 1] = key;
        }
    }

    void shellSort(T* array, int lo, int hi) {
        for (int k = 0; k < incsLen; k++) {
            for (int i = k + lo; i < hi; i++) {
                T v = array[i];
                
                int j = i;
                for (; j >= k && compare(array[j - k], v) > 0; j -= k)
                    array[j] = array[j - k];
                array[j] = v;
            }
        }
    }

    int partition(T* array, int a, int b, int p) {
        int i = a - 1,
            j = b;

        while (true) {
            i++;
            while (i <  b && compare(array[i], array[p]) < 0) i++;

            j--;
            while (j >= a && compare(array[j], array[p]) > 0) j--;

            if (i < j) swap(array, i, j);
            else       return j;
        }
    }

    void compNSwap(T* array, int a, int b, int gap, int start) {
        if (compare(array[start + (a * gap)], array[start + (b * gap)]) > 0)
            swap(array, start + (a * gap), start + (b * gap));
    }

    void medianOfThree(T* array, int a, int b) {
        int m = a + (b - 1 - a) / 2;

        compNSwap(array, a, m, 1, 0);
        if (compare(array[m], array[b - 1]) > 0) {
            swap(array, m, b - 1);

            if (compare(array[a], array[m]) > 0) return;
        }

        swap(array, a, m);
    }

    void medianOfSixteen(T* array, int a, int b) {
        int gap = (b - 1 - a) / 16;

        for (int i = 0; i < medianOfSixteenSwapsLen; i += 2)
            compNSwap(array, medianOfSixteenSwaps[i], medianOfSixteenSwaps[i + 1], gap, a);

        swap(array, a, a + (8 * gap));
    }

    bool getSortedRuns(T* array, int start, int end) {
        bool reverseSorted = true,
                    sorted = true;

        for (int i = start; i < end - 1; i++) {
            if (compare(array[i], array[i + 1]) > 0)
                sorted = false;
            else reverseSorted = false;

            if ((!reverseSorted) && (!sorted)) return false;
        }

        if (reverseSorted && !sorted) {
            reverse(array, start, end);
            return true;
        }

        return sorted;
    }

    void medianOfSixteenAQSort(T* arr, int low, int high, int depthLimit) {
        while (high - low > 16) {
            if (getSortedRuns(arr, low, high)) return;
            if (depthLimit == 0) {
                maxHeapSort(arr, low, high);
                return;
            }

            medianOfThree(arr, low, high);
            int pi = partition(arr, low, high, low);

            int left  = pi   - low,
                right = high - (pi + 1);

            if ((left == 0 || right == 0) || (left / right >= 16 || right / left >= 16)) {
                if (high - low > 80) {
                    medianOfSixteen(arr, low, high);
                    pi = partition(arr, low + 1, high, low);
                } else {
                    shellSort(arr, low, high);
                    return;
                }
            }

            swap(arr, low, pi);

            depthLimit--;
            medianOfSixteenAQSort(arr, pi + 1, high, depthLimit);
            high = pi;
        }
        uncheckedInsertionSort(arr, low, high);
    }

    void sort(T* arr, int start, int end) {
        medianOfSixteenAQSort(arr, start, end, (int)(2 * log2(end - start)));
    }

    void multiSwap(T* array, int a, int b, int len) {
        for (int i = 0; i < len; i++)
            swap(array, a + i, b + i);
    }

    void multiSwapBW(T* array, int a, int b, int len) {
        for (int i = 0; i < len; i++)
            swap(array, a + len - i - 1, b + len - i - 1);
    }

    void insertTo(T* array, int from, int to) {
        T tmp = array[from];

        for (int i = from - 1; i >= to; i--)
            array[i + 1] = array[i];
        array[to] = tmp;
    }

    void insertToBW(T* array, int from, int to) {
        T tmp = array[from];

        for (int i = from; i < to; i++)
            array[i] = array[i + 1];
        array[to] = tmp;
    }

    int binarySearch(T* array, int a, int b, T value, bool left) {
        bool comp;
        while (a < b) {
            int m = (a + b) / 2;

            if (left) comp = compare(value, array[m]) <= 0;
            else      comp = compare(value, array[m]) <  0;

            if (comp) b = m;
            else      a = m + 1;
        }

        return a;
    }

    void rotate(T* array, int a, int m, int b) {
        while (b - m > 1 && m - a > 1) {
            if (b - m < m - a) {
                multiSwap(array, a, m, b - m);
                a += b - m;
            } else {
                multiSwapBW(array, a, b - (m - a), m - a);
                b -= m - a;
            }
        }

        if      (b - m == 1) insertTo(array, m, a);
        else if (m - a == 1) insertToBW(array, a, b - 1);
    }

    int triSearch(T* arr, int l, int h, T val) {
        int m = l + ((h - l) / 2);

        if      (compare(val, arr[l]) < 0) return l;
        else if (compare(val, arr[h]) < 0) {
            if  (compare(val, arr[m]) < 0)
                 return triSearch(arr, l + 1, m - 1, val);
            else return triSearch(arr, m + 1, h - 1, val);
        } else return h + 1;
    }

    void triInsertSort(T* array, int start, int end) {
        for (int i = start + 1; i < end; i++) {
            T tmp  = array[i];
            int lo = triSearch(array, start, i - 1, tmp);

            for (int j = i - 1; j >= lo; j--)
                array[j + 1] = array[j];
            array[lo] = tmp;
        }
    }

    void mergeUp(T* array, int leftStart, int rightStart, int end) {
        T* copied = new T[rightStart - leftStart];

        for (int i = 0; i < rightStart - leftStart; i++)
            copied[i] = array[i + leftStart];

        int left  = leftStart,
            right = rightStart;

        for (int nxt = 0; nxt < end - leftStart; nxt++) {
            if (left >= rightStart && right >= end) break;

            if (left < rightStart && right >= end)
                array[nxt + leftStart] = copied[(left++) - leftStart];
            else if (left >= rightStart && right < end) break;
            else if (compare(copied[left - leftStart], array[right]) <= 0)
                array[nxt + leftStart] = copied[(left++) - leftStart];
            else
                array[nxt + leftStart] = array[right++];
        }

        delete[] copied;
    }

    void mergeDown(T* array, int leftStart, int rightStart, int end) {
        T* copied = new T[end - rightStart];

        for (int i = 0; i < end - rightStart; i++)
            copied[i] = array[i + rightStart];

        int left  = rightStart - 1,
            right = end;

        for (int nxt = end - leftStart - 1; nxt >= 0; nxt--) {
            if (left <= leftStart && right <= rightStart) break;

            if (left < leftStart && right >= leftStart)
                array[leftStart + nxt] = copied[(right--) - rightStart - 1];
            else if ((left >= leftStart && right < leftStart) || right < rightStart + 1) break;
            else if (compare(array[left], copied[right - rightStart - 1]) <= 0)
                array[leftStart + nxt] = copied[(right--) - rightStart - 1];
            else
                array[leftStart + nxt] = array[left--];
        }

        delete[] copied;
    }

    bool checkBounds(T* array, int a, int m, int b) {
        if (compare(array[m - 1], array[m]) <= 0)
            return true;
        else if (compare(array[a], array[b - 1]) > 0) {
            rotate(array, a, m, b);
            return true;
        }
        return false;
    }

    void mergeInPlace(T* array, int a, int m, int b) {
        if (m - a <= b - m) {
            int i = a, j = m, k;
            while (i < j && j < b) {
                if (compare(array[i], array[j]) > 0) {
                    k = binarySearch(array, j, b, array[i], true);
                    rotate(array, i, j, k);
                    i += k - j;
                    j = k;
                } else i++;
            }
        } else {
            int i = m - 1, j = b - 1, k;
            while (j > i && i >= a) {
                if (compare(array[i], array[j]) > 0) {
                    k = binarySearch(array, a, i, array[j], false);
                    rotate(array, k, i + 1, j + 1);
                    j -= (i + 1) - k;
                    i = k - 1;
                } else j--;
            }
        }
    }

    void merge(T* array, int a, int m, int b) {
        if (checkBounds(array, a, m, b)) return;

        b = binarySearch(array, m,     b, array[m - 1], true);
        a = binarySearch(array, a, m - 1, array[m],     false);

        if (b - m < m - a) {
            if (b - m <= 8)
                mergeInPlace(array, a, m, b);
            else
                mergeDown(array, a, m, b);
        } else {
            if (m - a <= 8)
                mergeInPlace(array, a, m, b);
            else
                mergeUp(array, a, m, b);
        }
    }

    bool getReversedRuns(T* array, int start, int end, int limit) {
        int i = start;
        for (; i < end - 1; i++)
            if (compare(array[i], array[i + 1]) <= 0) break;

        if (i - start > limit) reverse(array, start, i);

        return (i == end);
    }

    void stableSort(T* array, int start, int end) {
        if (getReversedRuns(array, start, end, 8)) return;
        if (end - start > 32) {
            int m = start + ((end - start) / 2);
            
            stableSort(array, start, m);
            stableSort(array,     m, end);

            merge(array, start, m, end);
        } else triInsertSort(array, start, end);
    }

    MinMaxPair<T> findMinMax(T* array, int a, int b) {
        MinMaxPair<T> minMax = MinMaxPair<T>{array[a], array[a]};

        for (int i = a + 1; i < b; i++) {
            if      (compare(array[i], minMax.min) < 0)
                minMax.min = array[i];
            else if (compare(array[i], minMax.max) > 0)
                minMax.max = array[i];
        }

        return minMax;
    }

    void featureSort(T* array, int a, int b) {
        MinMaxPair<T> minMax = findMinMax(array, a, b);

        float constant;
        if (minMax.min.value < 0) constant = (b - a) / (minMax.max.value - minMax.min.value + 4);
        else                constant = (b - a) / (minMax.max.value + 4);
        
        std::vector<T>* aux = new std::vector<T>[b - a + 1];

        for (int i = a; i < b; i++)
            aux[(int)((float)(array[i].value - minMax.min.value) * constant)].push_back(array[i]);

        for (int i = 0; i < b - a; i++)
            stableSort(aux[i].data(), 0, aux[i].size());

        for (int i = 0, r = a; i < b - a; i++)
            for (int j = 0; j < aux[i].size(); j++, r++)
                array[r] = aux[i][j];

        delete[] aux;
    }
};
