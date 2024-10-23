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
void counting_sort(vector<unsigned int>& arr, unsigned int exp) {
    unsigned int n = arr.size();
    vector<unsigned int> sorted_arr(n);
    int count[10] = {0};

    for (int i = 0; i < n; i++) {
        count[(arr[i] / exp) % 10]++;
    }

    
    for (int i = 1; i < 10; i++) {
        count[i] += count[i - 1];
    }


    for (int i = n - 1; i >= 0; i--) {
        sorted_arr[count[(arr[i] / exp) % 10] - 1] = arr[i];
        count[(arr[i] / exp) % 10]--;
    }

    for (int i = 0; i < n; i++) {
        arr[i] = sorted_arr[i];
    }
}

// Function to perform distributed radix sort
void radix_sort_helper(vector<unsigned int>& arr, unsigned int n, unsigned int rank, unsigned int p) {

    unsigned int local_max = *std::max_element(arr.begin(), arr.end());

    // Iterate over each digit (LSD to MSD)
    for (unsigned int exp = 1; local_max / exp > 0; exp *= 10) {
        // Perform counting sort on the current digit
        counting_sort(arr, exp);
    }

}

//Merge function
vector<unsigned int> merge(const vector<unsigned int>& left, const vector<unsigned int>& right) {
    vector<unsigned int> result;
    int i = 0, j = 0;

    while (i < left.size() && j < right.size()) {
        if (left[i] < right[j]) {
            result.push_back(left[i]);
            i++;
        } else {
            result.push_back(right[j]);
            j++;
        }
    }

    while (i < left.size()) {
        result.push_back(left[i]);
        i++;
    }
    while (j < right.size()) {
        result.push_back(right[j]);
        j++;
    }

    return result;
}

void radix_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    

    unsigned int elem_per_proc = n / p;
    
    vector<unsigned int> recvbuf(elem_per_proc);
    
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    
    MPI_Scatter(arr, elem_per_proc, MPI_UNSIGNED, 
                recvbuf.data(), elem_per_proc, MPI_UNSIGNED, 
                0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    // Perform distributed radix sort
    radix_sort_helper(recvbuf, elem_per_proc, rank, p);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");


    //Gather all the local arrays into a global array
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    // Gather the sorted subarrays into global sorted
    vector<unsigned int> global(n);

    MPI_Gather(recvbuf.data(), elem_per_proc, MPI_UNSIGNED, global.data(), elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);
    
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    
    //Merge all the subarrays into arr

    if (rank == 0) {
       vector<vector<unsigned int>> parts(p);
        for (int i = 0; i < p; ++i)
            parts[i] = vector<unsigned int>(global.begin() + i * elem_per_proc, global.begin() + (i+1) * elem_per_proc);

        CALI_MARK_BEGIN("comp");
        CALI_MARK_BEGIN("comp_large");
        //Merge sorted arrays
        vector<unsigned int> sorted = parts[0];
        for (int i = 1; i < parts.size(); ++i) {
            sorted = merge(sorted, parts[i]);
        }
      
      // Copy final sorted result to arr
      memcpy(arr, sorted.data(), sorted.size() * sizeof(unsigned int));

      CALI_MARK_END("comp_large");
      CALI_MARK_END("comp");
    }
     

}