#!/bin/bash
# -*- coding: utf-8 -*-

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# MT
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=0 && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/MT $@
else
    echo "Compile failed"
    exit 1
fi

# PI
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=1 && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/PI $@
else
    echo "Compile failed"
    exit 1
fi

# PI_CACHED
cmake . -DPR_TRIANGLE_USE_CACHE=ON -DPR_TRIANGLE_INTERSECTION_METHOD=1 && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/PI_CACHED -b $@
else
    echo "Compile failed"
    exit 1
fi
