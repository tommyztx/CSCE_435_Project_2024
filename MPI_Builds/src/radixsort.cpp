#include "radixsort.h"
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <mpi.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>

using namespace std;

// Helper function for counting sort by digit
void counting_sort_digit(unsigned int* arr, unsigned int n, unsigned int exp) {
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    
    unsigned int output[n];
    unsigned int count[10] = {0};

    // Store count of occurrences of digits
    for (unsigned int i = 0; i < n; i++) {
        unsigned int digit = (arr[i] / exp) % 10;
        count[digit]++;
    }

    // Change count so that it contains actual position of the digits
    for (unsigned int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }

    // Build the output array
    for (int i = n - 1; i >= 0; i--) {
        unsigned int digit = (arr[i] / exp) % 10;
        output[count[digit] - 1] = arr[i];
        count[digit]--;
    }

    // Copy the output array to arr[]
    for (unsigned int i = 0; i < n; i++) {
        arr[i] = output[i];
    }
    
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
}

// Function to perform distributed radix sort
void radix_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {


    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    
    unsigned int max_val;
    unsigned int local_max = *std::max_element(arr, arr + n / p);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    // Find global max using MPI_Allreduce
    MPI_Allreduce(&local_max, &max_val, 1, MPI_UNSIGNED, MPI_MAX, MPI_COMM_WORLD);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");
    // Iterate over each digit (LSD to MSD)
    for (unsigned int exp = 1; max_val / exp > 0; exp *= 10) {
        // Perform counting sort on the current digit
        counting_sort_digit(arr, n / p, exp);

        CALI_MARK_BEGIN("comm");
        CALI_MARK_BEGIN("comm_large");
        // Gather sorted arrays from each process
        unsigned int* global_sorted = nullptr;
        if (rank == 0) {
            global_sorted = new unsigned int[n];
        }

        MPI_Gather(arr, n / p, MPI_UNSIGNED, global_sorted, n / p, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

        // Redistribute the sorted array for the next digit's iteration
        MPI_Scatter(global_sorted, n / p, MPI_UNSIGNED, arr, n / p, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

        if (rank == 0) {
            delete[] global_sorted;
        }
        
        CALI_MARK_END("comm_large");
        CALI_MARK_END("comm");
    }

}

void radix_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    

    unsigned int elem_per_proc = n / p;
    
    unsigned int* recvbuf = new unsigned int[elem_per_proc];
    
     CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    
    // Use MPI_IN_PLACE for rank 0 to avoid buffer aliasing
    MPI_Scatter(arr, elem_per_proc, MPI_UNSIGNED, 
                recvbuf, elem_per_proc, MPI_UNSIGNED, 
                0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // Perform distributed radix sort
    radix_sort_helper(recvbuf, elem_per_proc, rank, p);
    


    // Wait for all processes to finish sorting
    MPI_Barrier(MPI_COMM_WORLD);

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    // Gather the sorted array
    if (rank == 0) {
        // Allocate temporary buffer to gather all sorted data
        unsigned int* global_sorted = new unsigned int[n];
        MPI_Gather(recvbuf, elem_per_proc, MPI_UNSIGNED, global_sorted, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
        // Copy gathered data back to arr
        memcpy(arr, global_sorted, n * sizeof(unsigned int));
        delete[] global_sorted;
    } else {
        MPI_Gather(recvbuf, elem_per_proc, MPI_UNSIGNED, nullptr, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    }
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    
    delete[] recvbuf;

}