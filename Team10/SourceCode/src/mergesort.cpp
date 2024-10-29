#include "mergesort.h"

// Merges two sorted arrays into a single sorted array
void merge(unsigned int* arr, unsigned int* left, unsigned int left_size, unsigned int* right, unsigned int right_size) {
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    unsigned int i = 0, j = 0, k = 0;

    while (i < left_size && j < right_size) {
        if (left[i] <= right[j]) {
            arr[k++] = left[i++];
        } else {
            arr[k++] = right[j++];
        }
    }

    while (i < left_size) {
        arr[k++] = left[i++];
    }

    while (j < right_size) {
        arr[k++] = right[j++];
    }

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

}

// Performs sequential merge sort
void sequential_merge_sort(unsigned int* arr, unsigned int size) {


    if (size <= 1) {
        return;
    }

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    unsigned int mid = size / 2;

    unsigned int* left = new unsigned int[mid];
    unsigned int* right = new unsigned int[size - mid];

    for (unsigned int i = 0; i < mid; i++) {
        left[i] = arr[i];
    }
    for (unsigned int i = mid; i < size; i++) {
        right[i - mid] = arr[i];
    }

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");
    
    sequential_merge_sort(left, mid);
    sequential_merge_sort(right, size - mid);

    merge(arr, left, mid, right, size - mid);

    delete[] left;
    delete[] right;
}

void merge_sort(unsigned int* arr, unsigned int size) {
    int rank, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    // Determine the portion size for each process
    unsigned int local_size = size / num_procs;
    unsigned int* local_array = new unsigned int[local_size];

    // Scatter the array segments to each process
    MPI_Scatter(arr, local_size, MPI_UNSIGNED, local_array, local_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");



    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");  
    
    // Each process sorts its local segment
    sequential_merge_sort(local_array, local_size);

    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // Gather the sorted segments back at the root process

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    unsigned int* gathered_array = nullptr;
    if (rank == 0) {
        gathered_array = new unsigned int[size];
    }
    MPI_Gather(local_array, local_size, MPI_UNSIGNED, gathered_array, local_size, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    // Root process performs the final merging of sorted segments
    if (rank == 0) {
        unsigned int* temp = new unsigned int[size];
        unsigned int segment_size = local_size;

        // Perform initial merging step for the gathered segments
        unsigned int* current_array = gathered_array;
        unsigned int* next_array = temp;

        // Merge segments two by two until we get the final sorted array
        while (segment_size < size) {
            for (unsigned int i = 0; i < size; i += 2 * segment_size) {
                unsigned int left_size = (i + segment_size < size) ? segment_size : (size - i);
                unsigned int right_size = ((i + 2 * segment_size) < size) ? segment_size : (size - i - left_size);

                // Merging the two segments
                if (right_size > 0) {
                    merge(next_array + i, current_array + i, left_size, current_array + i + left_size, right_size);
                } else {
                    // If there's no right segment, just copy the left segment
                    std::copy(current_array + i, current_array + i + left_size, next_array + i);
                }
            }

            // Swap arrays
            std::swap(current_array, next_array);
            segment_size *= 2;
        }

        // If current_array is not the original gathered_array, copy back to gathered_array
        if (current_array != gathered_array) {
            std::copy(current_array, current_array + size, gathered_array);
        }

        std::copy(gathered_array, gathered_array + size, arr);

      

        delete[] temp;
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    //Clean up
    delete[] local_array;
    if (rank == 0) {
        delete[] gathered_array;
    }

    //MPI_Barrier(MPI_COMM_WORLD);
}
