#include "samplesort.h"

void collect_sample(unsigned int* arr, unsigned int n, unsigned int* sample, unsigned int sample_size) {
    for (unsigned int i = 0; i < k; ++i) {
        unsigned int rand_ind = rand() % (n / k) + i * (n / k);
        sample[i] = arr[rand_ind];
    }
}



void insertion_sort(unsigned int* arr, unsigned int n) {
    for (int i = 1; i < n; ++i) {
        unsigned int temp = arr[i];

        int hole = i;
        while (hole > 0 && hole[i] < hole[i - 1]) {
            arr[hole] = arr[hole - 1];
            --hole;
        }

        arr[hole] = temp;
    }
}



void sample_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    // Collect Personal Sample
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    unsigned int my_sample[k];
    collect_sample(arr, n / p, my_sample, k);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");


    // Gather pivot candidates
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    unsigned int* sample = nullptr;
    if (rank == 0) {
        sample = new unsigned int[p * k];
    }

    MPI_Gather(my_sample, k, MPI_UNSIGNED, sample, p * k, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");
    

    // Sort pivot candidates and select pivots
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    unsigned int* pivots = new unsigned int[p - 1];
    if (rank == 0) {
        insertion_sort(sample, p * k);

        for (unsigned int i = 1; i < p; ++i) {
            pivots[i - 1] = sample[i * k];
        }
    }

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");


    // Disperse pivots
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    MPI_Bcast(pivots, p - 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    printf("SAMPLE SORT\n");
}



void sample_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    // Distribute array amongst processes
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    unsigned int elem_per_proc = n / p;
    if (rank > 0) {
        arr = new unsigned int[elem_per_proc];
    }
    
    MPI_Scatter(arr, elem_per_proc, MPI_UNSIGNED, arr, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");


    // Perform distributed sample sort
    sample_sort_helper(arr, n, rank, p);


    // Deallocate array in helper processes
    if (rank > 0) {
        delete[] arr;
    }
}