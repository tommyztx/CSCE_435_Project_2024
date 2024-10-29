#ifndef MERGE_SORT
#define MERGE_SORT


#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <algorithm>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

// TODO: Edit function signature and add helper functions as necessary

void merge_sort(unsigned int* arr, unsigned int n);

void merge(unsigned int* arr, unsigned int* left, unsigned int left_size, unsigned int* right, unsigned int right_size);

void sequential_merge_sort(unsigned int* arr, unsigned int size);

//void parallel_merge_sort(unsigned int* arr, unsigned int size, int rank, int num_procs);



// TODO: Edit function signature and add helper functions as necessary
// const unsigned int k = 4;

// void collect_sample(unsigned int* arr, unsigned int n, unsigned int* sample, unsigned int sample_size);

// void insertion_sort(unsigned int* arr, unsigned int n);

// unsigned int* grow_array(unsigned int* arr, unsigned int n, unsigned int* old_cap, unsigned int new_cap);

// void sample_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

// void sample_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);




#endif