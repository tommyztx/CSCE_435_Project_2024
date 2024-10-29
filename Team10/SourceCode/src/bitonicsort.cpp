#include "bitonicsort.h"

void compAndSwap(unsigned int a[], unsigned int i, unsigned int j, bool dir)
{
    if (dir == (a[i] > a[j]))
    {
        std::swap(a[i], a[j]);
    }
}

void bitonic_merge(unsigned int arr[], unsigned int low, unsigned int cnt, bool dir)
{
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        for (unsigned int i = low; i < low + k; i++)
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

    bitonic_sort_helper(local_arr, 0, elem_per_proc, 1);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    int phases = 0;
    int proc_count = p;
    while (proc_count > 1)
    {
        proc_count /= 2;
        phases++;
    }

    for (int phase = 1; phase <= phases; phase++)
    {
        int group_size = 1;
        for (int j = 0; j < phase; j++)
        {
            group_size *= 2;
        }

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

        MPI_Sendrecv(local_arr, elem_per_proc, MPI_UNSIGNED, partner, 0,
                     recv_buffer, elem_per_proc, MPI_UNSIGNED, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (rank < partner)
        {
            for (unsigned int l = 0; l < elem_per_proc; l++)
            {
                local_arr[l] = std::min(local_arr[l], recv_buffer[l]);
            }
        }
        else
        {
            for (unsigned int l = 0; l < elem_per_proc; l++)
            {
                local_arr[l] = std::max(local_arr[l], recv_buffer[l]);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        delete[] recv_buffer;
    }

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");

    MPI_Gather(local_arr, elem_per_proc, MPI_UNSIGNED, arr, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    delete[] local_arr;
}