#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <time.h>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#include "src/bitonicsort.h"
#include "src/samplesort.h"
#include "src/mergesort.h"
#include "src/radixsort.h"

// Swaps perturbed_ratio of the elements in the array
void perturb_array(unsigned int* arr, unsigned int n, float perturbed_ratio) {
    unsigned int num_swaps = (unsigned int) (n * perturbed_ratio);
    
    for (unsigned int swap_no = 0; swap_no < num_swaps; ++swap_no) {
        unsigned int first_ind = rand() % n;
        unsigned int second_ind = rand() % n;

        while (first_ind == second_ind) {
            second_ind = rand() % n;
        }

        unsigned int temp = arr[first_ind];
        arr[first_ind] = arr[second_ind];
        arr[second_ind] = temp;
    }
}

int main(int argc, char *argv[]) {
    CALI_CXX_MARK_FUNCTION;

    // Check for proper number of command-line arguments
    if (argc != 4) {
        printf("Usage: sort <sort_name> <input_type> <array_size>\n");
        exit(1);
    }

    // Retrieve command-line arguments
    std::string algorithm(argv[1]);
    std::string input_type(argv[2]);
    unsigned int input_size = std::stoi(argv[3]);

    int num_procs, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    if (num_procs < 2 ) {
        printf("Need at least two MPI tasks. Quitting...\n");
        int rc;
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }

    // Create caliper ConfigManager object
    cali::ConfigManager mgr;
    mgr.start();

    // Log basic metadata
    adiak::init(NULL);
    adiak::launchdate();    // launch date of the job
    adiak::libraries();     // Libraries used
    adiak::cmdline();       // Command line used to launch the job
    adiak::clustername();   // Name of the cluster
    adiak::value("algorithm", algorithm.c_str()); // The name of the algorithm you are using (e.g., "merge", "bitonic")
    adiak::value("programming_model", "mpi"); // e.g. "mpi"
    adiak::value("data_type", "unsigned int"); // The datatype of input elements (e.g., double, int, float)
    adiak::value("size_of_data_type", sizeof(unsigned int)); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
    adiak::value("input_size", input_size); // The number of elements in input dataset (1000)
    adiak::value("input_type", input_type.c_str()); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
    adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)
    adiak::value("group_num", 10); // The number of your group (integer, e.g., 1, 10)
    adiak::value("implementation_source", "handwritten"); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").

    // Build input array
    srand(time(NULL));
    unsigned int* input_array = nullptr;
    if (rank == 0) {
        CALI_MARK_BEGIN("data_init_runtime");

        input_array = new unsigned int[input_size];
        
        if (input_type == "Sorted") {
            for (unsigned int i = 0; i < input_size; ++i) {
                input_array[i] = i;
            }
        }
        else if (input_type == "ReverseSorted") {
            for (unsigned int i = 0; i < input_size; ++i) {
                input_array[i] = input_size - i;
            }
        }
        else if (input_type == "Random") {
            for (unsigned int i = 0; i < input_size; ++i) {
                input_array[i] = i;
            }

            perturb_array(input_array, input_size, 1);
        }
        else if (input_type == "1_perc_perturbed") {
            for (unsigned int i = 0; i < input_size; ++i) {
                input_array[i] = i;
            }

            perturb_array(input_array, input_size, 0.01);
        }
        else {
            printf("ERROR: Invalid input type provided!!!\n");
            exit(1);
        }

        CALI_MARK_END("data_init_runtime"); 
    }

    // Call proper sort
    if (algorithm == "bitonic") {
        bitonic_sort(input_array, input_size);
        adiak::value("scalability", "strong or weak?"); // The scalability of your algorithm. choices: ("strong", "weak")
    }
    else if (algorithm == "sample") {
        sample_sort(input_array, input_size, rank, num_procs);
        adiak::value("scalability", "strong or weak?"); // The scalability of your algorithm. choices: ("strong", "weak")
    }
    else if (algorithm == "merge") {
        merge_sort(input_array, input_size);
        adiak::value("scalability", "strong or weak?"); // The scalability of your algorithm. choices: ("strong", "weak")
    }
    else if (algorithm == "radix") {
        radix_sort(input_array, input_size);
        adiak::value("scalability", "strong or weak?"); // The scalability of your algorithm. choices: ("strong", "weak")
    }
    else {
        printf("ERROR: Invalid sorting algorithm provided!!!\n");
        exit(1);
    }

    // Check for array correctness
    if (rank == 0) {
        CALI_MARK_BEGIN("correctness_check"); 
        
        bool sorted = true;
        for (unsigned int i = 1; i < input_size; ++i) {
            if (input_array[i] < input_array[i - 1]) {
                sorted = false;
                break;
            }
        }

        if (sorted) {
            printf("Array correctly sorted.\n");
        }
        else {
            printf("ERROR: Array not correctly sorted!!!\n");

            // printf("Array:");
            // for (unsigned int i = 0; i < input_size; ++i) {
            //     printf(" %u", input_array[i]);
            // }
            // printf("\n");
        }

        delete[] input_array;

        CALI_MARK_END("correctness_check");
    }

    // Flush Caliper output before finalizing MPI
    mgr.stop();
    mgr.flush();

    MPI_Finalize();

    return 0;
}