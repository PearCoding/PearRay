#!/bin/bash
# -*- coding: utf-8 -*-

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# MT SINGLE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=0 -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/MT_S $@
else
    echo "Compile failed"
    exit 1
fi

# PI SINGLE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON  && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/PI_S $@
else
    echo "Compile failed"
    exit 1
fi

# PI_CACHED SINGLE
cmake . -DPR_TRIANGLE_USE_CACHE=ON -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON  && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/PI_CACHED_S $@
else
    echo "Compile failed"
    exit 1
fi

# MT PACKAGE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=0 -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/MT_P $@
else
    echo "Compile failed"
    exit 1
fi

# PI PACKAGE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF  && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/PI_P $@
else
    echo "Compile failed"
    exit 1
fi

# PI_CACHED PACKAGE
cmake . -DPR_TRIANGLE_USE_CACHE=ON -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF  && ninja -j4
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o benchmarks/PI_CACHED_P -b $@
else
    echo "Compile failed"
    exit 1
fi
