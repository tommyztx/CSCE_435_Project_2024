#ifndef RADIX_SORT
#define RADIX_SORT

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

// TODO: Edit function signature and add helper functions as necessary

void counting_sort_digit(unsigned int* arr, unsigned int n, unsigned int exp);

void radix_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

void radix_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p);

#endif