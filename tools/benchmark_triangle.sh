#!/bin/bash
# -*- coding: utf-8 -*-

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CURR_DIR="$( pwd )"

cd $THIS_DIR/../build

# MT SINGLE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=0 -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON && ninja -j4 pearray
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/MT_S -e bin/pearray $@
else
    echo "Compile failed"
    exit 1
fi

# PI SINGLE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON  && ninja -j4 pearray
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/PI_S -e bin/pearray $@
else
    echo "Compile failed"
    exit 1
fi

# PI_CACHED SINGLE
cmake . -DPR_TRIANGLE_USE_CACHE=ON -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON  && ninja -j4 pearray
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/PI_CACHED_S -e bin/pearray $@
else
    echo "Compile failed"
    exit 1
fi

# MT PACKAGE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=0 -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF && ninja -j4 pearray
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/MT_P -e bin/pearray $@
else
    echo "Compile failed"
    exit 1
fi

# PI PACKAGE
cmake . -DPR_TRIANGLE_USE_CACHE=OFF -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF  && ninja -j4 pearray
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/PI_P -e bin/pearray $@
else
    echo "Compile failed"
    exit 1
fi

# PI_CACHED PACKAGE
cmake . -DPR_TRIANGLE_USE_CACHE=ON -DPR_TRIANGLE_INTERSECTION_METHOD=1 -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF  && ninja -j4 pearray
if [ $? -eq 0 ]; then
    $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/PI_CACHED_P -e bin/pearray -b $@
else
    echo "Compile failed"
    exit 1
fi

cd $CURR_DIR