#include "samplesort.h"

void collect_sample(unsigned int* arr, unsigned int n, unsigned int* sample, unsigned int sample_size) {
    for (unsigned int i = 0; i < sample_size; ++i) {
        unsigned int rand_ind = rand() % (n / sample_size) + i * (n / sample_size);
        sample[i] = arr[rand_ind];
    }
}



void insertion_sort(unsigned int* arr, unsigned int n) {
    for (unsigned int i = 1; i < n; ++i) {
        unsigned int temp = arr[i];

        unsigned int hole = i;
        while (hole > 0 && temp < arr[hole - 1]) {
            arr[hole] = arr[hole - 1];
            --hole;
        }

        arr[hole] = temp;
    }
}



unsigned int* grow_array(unsigned int* arr, unsigned int n, unsigned int* old_cap, unsigned int new_cap) {
    unsigned int resize = 2 * (*old_cap);
    while (resize < new_cap) {
        resize *= 2;
    }
    *old_cap = resize;

    unsigned int* temp_arr = new unsigned int[resize];
    for (unsigned int i = 0; i < n; ++i) {
        temp_arr[i] = arr[i];
    }

    delete[] arr;
    return temp_arr;
}



void sample_sort_helper(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    // Collect Personal Sample
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    unsigned int my_sample[k];
    collect_sample(arr, n / p, my_sample, k);

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");


    // Gather pivot candidates
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    unsigned int* sample = nullptr;
    if (rank == 0) {
        sample = new unsigned int[p * k];
    }

    MPI_Gather(my_sample, k, MPI_UNSIGNED, sample, k, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");
    

    // Sort pivot candidates and select pivots
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    unsigned int* pivots = new unsigned int[p - 1];
    if (rank == 0) {
        insertion_sort(sample, p * k);

        for (unsigned int i = 1; i < p; ++i) {
            pivots[i - 1] = sample[i * k];
        }
    }

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");


    // Disperse pivots
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");

    MPI_Bcast(pivots, p - 1, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");


    // Sort elements into local buckets
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");

    std::vector<unsigned int>* buckets = new std::vector<unsigned int>[p]{};
    for (int i = 0; i < p; ++i) {
        buckets[i].reserve(n / (p * p));
    }

    for (unsigned int i = 0; i < n / p; ++i) {
        unsigned int elem = arr[i];
        bool found_bucket = false;

        for (unsigned int bucket = 0; bucket < p - 1; ++bucket) {
            if (elem < pivots[bucket]) {
                found_bucket = true;
                buckets[bucket].push_back(elem);
                break;
            }
            else if (elem == pivots[bucket]) { // Assumes there are no duplicate elements
                found_bucket = true;
                break;
            }
        }

        if (!found_bucket) {
            buckets[p - 1].push_back(elem);
        }
    }

    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");


    // Distribute buckets to proper process
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    unsigned int my_bucket_size = 0;
    unsigned int curr_size = 0;
    MPI_Status status;
    if (rank == 0) {
        memcpy(arr, buckets[0].data(), buckets[0].size() * sizeof(unsigned int));
        my_bucket_size += buckets[0].size();

        for (unsigned int i = 1; i < p; ++i) {
            MPI_Recv(&curr_size, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &status);

            if (curr_size > 0) {
                MPI_Recv(arr + my_bucket_size, curr_size, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &status);
                my_bucket_size += curr_size;
            }
        }

        for (unsigned int i = 1; i < p; ++i) {
            curr_size = buckets[i].size();
            MPI_Send(&curr_size, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);

            if (curr_size > 0) {
                MPI_Send(buckets[i].data(), curr_size, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
            }
        }
    }

    else {
        curr_size = buckets[0].size();
        MPI_Send(&curr_size, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);

        if (curr_size > 0) {
            MPI_Send(buckets[0].data(), curr_size, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
        }

        unsigned int my_bucket_cap = n / p;
        for (unsigned int i = 1; i < p; ++i) {
            if (i == rank) {
                if (buckets[i].size() > my_bucket_cap) {
                    arr = grow_array(arr, 0, &my_bucket_cap, buckets[i].size());
                }

                memcpy(arr, buckets[i].data(), buckets[i].size() * sizeof(unsigned int));
                my_bucket_size += buckets[i].size();
                
                for (unsigned int j = 0; j < p; ++j) {
                    if (i == j) {
                        continue;
                    }

                    MPI_Recv(&curr_size, 1, MPI_UNSIGNED, j, 0, MPI_COMM_WORLD, &status);

                    if (curr_size > 0) {
                        if (my_bucket_size + curr_size > my_bucket_cap) {
                            arr = grow_array(arr, my_bucket_size, &my_bucket_cap, my_bucket_size + curr_size);
                        }

                        MPI_Recv(arr + my_bucket_size, curr_size, MPI_UNSIGNED, j, 0, MPI_COMM_WORLD, &status);
                        my_bucket_size += curr_size;
                    }
                }
            }

            else {
                curr_size = buckets[i].size();
                MPI_Send(&curr_size, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);

                if (curr_size > 0) {
                    MPI_Send(buckets[i].data(), curr_size, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD);
                }
            }
        }
    }

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");


    // Sort my bucket
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");

    if (my_bucket_size > 1) {
        insertion_sort(arr, my_bucket_size);
    }

    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");


    // Consolidate buckets into original array
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    if (rank == 0) {
        arr[my_bucket_size++] = pivots[0]; // Remove is there are duplicate elements
        for (unsigned int i = 1; i < p; ++i) {
            MPI_Recv(&curr_size, 1, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &status);

            if (curr_size > 0) {
                MPI_Recv(arr + my_bucket_size, curr_size, MPI_UNSIGNED, i, 0, MPI_COMM_WORLD, &status);
                my_bucket_size += curr_size;
            }

            if (i < p - 1) {
                arr[my_bucket_size++] = pivots[i]; // Remove is there are duplicate elements
            }
        }
    }

    else {
        MPI_Send(&my_bucket_size, 1, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);

        if (my_bucket_size > 0) {
            MPI_Send(arr, my_bucket_size, MPI_UNSIGNED, 0, 0, MPI_COMM_WORLD);
        }
    }

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");


    // Deallocate any allocated memory
    delete[] pivots;
    delete[] buckets;
    if (rank == 0) {
        delete[] sample;
    }
    else {
        delete[] arr;
    }
}



void sample_sort(unsigned int* arr, unsigned int n, unsigned int rank, unsigned int p) {
    // Distribute array amongst processes
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");

    unsigned int elem_per_proc = n / p;
    if (rank > 0) {
        arr = new unsigned int[elem_per_proc];
    }
    
    MPI_Scatter(arr, elem_per_proc, MPI_UNSIGNED, (rank == 0) ? MPI_IN_PLACE : arr, elem_per_proc, MPI_UNSIGNED, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");


    // Perform distributed sample sort
    sample_sort_helper(arr, n, rank, p);


    // Wait for all processes to finish sorting
    MPI_Barrier(MPI_COMM_WORLD);
}