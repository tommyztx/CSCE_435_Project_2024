#include "samplesort.h"

void sample_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    printf("SAMPLE SORT\n");
}

void sample_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    // Allocate array in helper processes
    unsigned int elem_per_proc = n / p;
    if (rank > 0) {
        arr = new unsigned int[elem_per_proc];
    }

    // Distribute array amongst processes
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatter(arr, elem_per_proc, MPI_UNSIGNED, arr, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    // Perform distributed sample sort
    sample_sort_helper(arr, n, rank, p)

    // Deallocate array in helper processes
    if (rank > 0) {
        delete[] arr;
    }
}