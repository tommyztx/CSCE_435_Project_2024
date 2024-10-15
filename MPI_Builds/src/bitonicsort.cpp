#include "bitonicsort.h"
#include <algorithm>
#include <cmath>

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

void bitonic_sort_seq(unsigned int *arr, unsigned int low, unsigned int cnt, bool dir)
{
    if (cnt > 1)
    {
        unsigned int k = cnt / 2;
        bitonic_sort_seq(arr, low, k, true);
        bitonic_sort_seq(arr, low + k, k, false);
        bitonic_merge(arr, low, cnt, dir);
    }
}

void bitonic_sort_parallel(unsigned int *arr, unsigned int n, unsigned int rank, unsigned int p)
{
    unsigned int local_n = n / p; // Each process gets `n / p` elements

    // Allocate memory for local array on all processes
    unsigned int *local_arr = new unsigned int[local_n];

    // Scatter the array to all processes
    if (rank == 0)
    {
        MPI_Scatter(MPI_IN_PLACE, local_n, MPI_UNSIGNED, arr, local_n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Scatter(nullptr, local_n, MPI_UNSIGNED, arr, local_n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    }

    // Perform local bitonic sort on each process
    bitonic_sort_seq(local_arr, 0, local_n, true);

    for (unsigned int i = 1; i <= std::log2(p); i++)
    {
        unsigned int partner = rank ^ (1 << (i - 1));
        unsigned int *partner_data = new unsigned int[local_n];
        MPI_Sendrecv(arr, local_n, MPI_UNSIGNED, partner, 0, partner_data, local_n, MPI_UNSIGNED, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        bool dir = (rank < partner);

        std::vector<unsigned int> merged_data(2 * local_n);

        if (dir)
        {
            std::merge(arr, arr + local_n, partner_data, partner_data + local_n, merged_data.begin(), std::less<unsigned int>());
        }
        else
        {
            std::merge(arr, arr + local_n, partner_data, partner_data + local_n, merged_data.begin(), std::greater<unsigned int>());
        }

        std::copy(merged_data.begin(), merged_data.begin() + local_n, arr);

        delete[] partner_data;
    }

    MPI_Gather((rank == 0) ? MPI_IN_PLACE : arr, local_n, MPI_UNSIGNED, arr, local_n, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    delete[] local_arr;
}

void bitonic_sort(unsigned int *arr, unsigned int n)
{
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