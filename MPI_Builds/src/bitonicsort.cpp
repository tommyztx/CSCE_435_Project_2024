#include "bitonicsort.h"

void bitonic_merge(unsigned int *arr, unsigned int low, unsigned int cnt, bool dir)
{
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        for (unsigned int i = low; i < low + k; i++)
        {
            if (dir == (arr[i] > arr[i + k]))
            {
                std::swap(arr[i], arr[i + k]);
            }
        }
        bitonic_merge(arr, low, k, dir);
        bitonic_merge(arr, low + k, k, dir);
    }
}

void bitonic_sort(unsigned int *arr, unsigned int low, unsigned int cnt, bool dir)
{
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        bitonic_sort(arr, low, k, true);
        bitonic_sort(arr, low + k, k, false);
        bitonic_merge(arr, low, cnt, dir);
    }
}