#!/bin/bash

dir=$PWD
files=("p_semaphore" "sequential" "thread_mutex" "thread_spin"
"p_trans" "tbb" "thread_semaphore" "thread_trans")

for element in ${files[@]}
do
    path=$dir/"progs"/$element
    cd $path
    make -f makefile
done
