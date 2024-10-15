#include "bitonicsort.h"

void compAndSwap(int a[], int i, int j, int dir)
{
    if (dir == (a[i] > a[j]))
    {
        std::swap(a[i], a[j]);
    }
}

void bitonic_merge(int arr[], int low, int cnt, int dir)
{
    if (cnt > 1)
    {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++)
        {
            compAndSwap(arr, i, i + k, dir);
        }
        bitonic_merge(arr, low, k, dir);
        bitonic_merge(arr, low + k, k, dir);
    }
}

void bitonic_sort_helper(unsigned int *arr, unsigned int low, unsigned int cnt, bool dir)
{
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        bitonic_sort_helper(arr, low, k, true);
        bitonic_sort_helper(arr, low + k, k, false);
        bitonic_merge(arr, low, cnt, dir);
    }
}

void bitonic_sort(unsigned int *arr, unsigned int n, unsigned int rank, unsigned int p)
{
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    unsigned int elem_per_proc = n / p;
    if (rank > 0)
    {
        arr = new unsigned int(elem_per_proc);
    }

    int rank, p;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    if ((n % p) != 0)
    {
        if (rank == 0)
        {
            printf("Array size must be divisible by the number of processors\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    bitonic_sort_parallel(arr, n, rank, p);

    if (rank == 0)
    {
        printf("BITONIC SORT COMPLETED\n");
    }
}