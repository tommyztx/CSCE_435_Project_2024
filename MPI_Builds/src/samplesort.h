#ifndef SAMPLE_SORT
#define SAMPLE_SORT

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <vector>
#include <cstring>
#include <algorithm>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

const unsigned int k = 4;

void collect_sample(unsigned int* arr, unsigned int n, unsigned int* sample, unsigned int sample_size);

// No longer using as std::sort() had SIGNIFICANTLY better performance
// void insertion_sort(unsigned int* arr, unsigned int n);

unsigned int* grow_array(unsigned int* arr, unsigned int n, unsigned int* old_cap, unsigned int new_cap);

void sample_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

void sample_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

#endif