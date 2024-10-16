#include "bitonicsort.h"
#include <cmath>

void compAndSwap(int a[], int i, int j, bool dir)
{
    if (dir == (a[i] > a[j]))
    {
        std::swap(a[i], a[j]);
    }
}

void bitonic_merge(int arr[], int low, int cnt, bool dir)
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

void bitonic_sort_helper(unsigned int arr[], unsigned int low, unsigned int cnt, bool dir)
{
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        bitonic_sort_helper(arr, low, k, true);
        bitonic_sort_helper(arr, low + k, k, false);
        bitonic_merge(arr, low, cnt, dir);
    }
}

void bitonic_sort(unsigned int arr[], unsigned int n, unsigned int rank, unsigned int p)
{

    unsigned int elem_per_proc = n / p;
    unsigned int *local_arr = new unsigned int[elem_per_proc];

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    MPI_Scatter(arr, elem_per_proc, MPI_UNSIGNED, local_arr, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    bitonic_sort_helper(local_arr, 0, elem_per_proc, true);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    for (int i = 1; i <= std::log2(p); i++)
    {
        int group_size = pow(2, i);
        int group_start = (rank / group_size) * group_size;
        int partner;

        if (rank < group_start + group_size / 2)
        {
            partner = rank + group_size / 2;
        }
        else
        {
            partner = rank - group_size / 2;
        }

        unsigned int *recv_buffer = new unsigned int[elem_per_proc];
        CALI_MARK_BEGIN("comm");
        CALI_MARK_BEGIN("comm_large");

        MPI_Sendrecv(local_arr, elem_per_proc, MPI_UNSIGNED, partner, 0,
                     recv_buffer, elem_per_proc, MPI_UNSIGNED, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        CALI_MARK_END("comm_large");
        CALI_MARK_END("comm");

        if (rank < partner)
        {
            for (unsigned int i = 0; i < elem_per_proc; i++)
            {
                local_arr[i] = std::min(local_arr[i], recv_buffer[i]);
            }
        }
        else
        {
            for (unsigned int i = 0; i < elem_per_proc; i++)
            {
                local_arr[i] = std::max(local_arr[i], recv_buffer[i]);
            }
        }
        delete[] recv_buffer;
    }

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");

    MPI_Gather(local_arr, elem_per_proc, MPI_UNSIGNED, arr, elem_per_procelem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    delete[] local_arr;
}