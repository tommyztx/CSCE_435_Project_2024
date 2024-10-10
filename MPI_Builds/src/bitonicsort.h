#ifndef BITONIC_SORT
#define BITONIC_SORT

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

// TODO: Edit function signature and add helper functions as necessary

void bitonic_sort(unsigned int* arr, unsigned int n);

#endif