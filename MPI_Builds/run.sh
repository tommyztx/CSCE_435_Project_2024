#!/bin/bash

# Grace properties
cores_per_node=48
mem_per_node=384

# Job file replacements
replace_name="SORT-NAME"
replace_type="INPUT-TYPE"
replace_size="INPUT-SIZE"
replace_proc="NUM-PROC"
replace_nodes="NUM-NODES"
replace_tasks="NUM-TASKS"
replace_mem="MEM-NEEDED"

# Command Line Argument Options
sort_names=("bitonic" "sample" "merge" "radix") # (bitonic sample merge radix)
input_types=("Sorted") # (Sorted ReverseSorted Random 1_perc_perturbed)
input_sizes=($((2**28))) # (2^16 2^18 2^20 2^22 2^24 2^26 2^28)
num_processes=(64) # (2 4 8 16 32 64 128 256 512 1024)

# Making sure everything is set up properly
make
mkdir -p Cali_Files
mkdir -p Output_Files

for name in "${sort_names[@]}"; do
    for type in "${input_types[@]}"; do
        for n in "${input_sizes[@]}"; do
            for p in "${num_processes[@]}"; do
                # Determine number of nodes needed
                num_nodes=$(($p / $cores_per_node))
                if (($p % $cores_per_node > 0))
                then
                    num_nodes=$(($num_nodes + 1));
                fi

                # Adjust number of nodes needed to be a power of 2
                for i in {0..10}; do
                    if (($num_nodes == 2 ** $i))
                    then
                        break
                    elif (($num_nodes < 2 ** $i))
                    then
                        num_nodes=$((2 ** $i))
                        break
                    fi
                done

                # Determine tasks per node needed
                num_tasks=$(($p / $num_nodes))
                if (($p % $num_nodes > 0))
                then
                    num_tasks=$(($num_tasks + 1));
                fi

                # Determine memory needed per node
                mem_needed=$(($num_tasks * $mem_per_node / $cores_per_node))

                # Create altered job file
                sed "s/$replace_name/$name/g; s/$replace_type/$type/g; s/$replace_size/$n/g; s/$replace_proc/$p/g; s/$replace_nodes/$num_nodes/; s/$replace_tasks/$num_tasks/; s/$replace_mem/$mem_needed/" mpi.grace_job > temp.grace_job
                
                # Dispatch job
                sbatch temp.grace_job $n $p
                
                # Remove altered job file
                rm temp.grace_job
            done
        done
    done
done