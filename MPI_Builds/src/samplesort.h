#ifndef SAMPLE_SORT
#define SAMPLE_SORT

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

// TODO: Edit function signature and add helper functions as necessary
const unsigned int k = 4;

void sample_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

void sample_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

#endif