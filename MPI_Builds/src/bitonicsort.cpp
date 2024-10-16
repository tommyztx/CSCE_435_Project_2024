#include "bitonicsort.h"

void compAndSwap(int a[], int i, int j, int dir)
{
    if (dir == (a[i] > a[j]))
    {
        std::swap(a[i], a[j]);
    }
}

void bitonic_merge(int arr[], int low, int cnt, bool dir)
{
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
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
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
}

void bitonic_sort_helper(unsigned int arr[], unsigned int low, unsigned int cnt, bool dir)
{
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        bitonic_sort_helper(arr, low, k, true);
        bitonic_sort_helper(arr, low + k, k, false);
        bitonic_merge(arr, low, cnt, dir);
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
}

void bitonic_sort(unsigned int arr[], unsigned int n, unsigned int rank, unsigned int p)
{

    unsigned int elem_per_proc = n / p;
    if (rank > 0)
    {
        unsigned int *local_arr = new unsigned int(elem_per_proc);
    }

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    MPI_Scatter(arr, local_size, MPI_UNSIGNED, , local_arr, local_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    bitonic_sort_helper(local_arr, 0, local_size, true);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    if (int i = 1; i <= log2(p); i++)
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

        unsigned int *recv_buffer = new unsigned int[local_size];
        CALI_MARK_BEGIN("comm");
        CALI_MARK_BEGIN("comm_large");

        MPI_Sendrecv(local_arr, local_size, MPI_UNSIGNED, partner, 0,
                     recv_buffer, local_size, MPI_UNSIGNED, partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        CALI_MARK_END("comm_large");
        CALI_MARK_END("comm");

        if (rank < partner)
        {
            for (unsigned int i = 0; i < local_size; i++)
            {
                local_arr[i] = std::min(local_arr[i], recv_buffer[i]);
            }
        }
        else
        {
            for (unsigned int i = 0; i < local_size; i++)
            {
                local_arr[i] = std::max(local_arr[i], recv_buffer[i]);
            }
        }
        delete[] recv_buffer;
    }

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");

    MPI_Gather(local_arr, local_size, MPI_UNSIGNED, arr, local_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    delete[] local_arr;
}