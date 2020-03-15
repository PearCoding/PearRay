#!/bin/bash
# -*- coding: utf-8 -*-

THIS_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
CURR_DIR="$( pwd )"

cd $THIS_DIR/../build

METHODS="MT PI PI_MEM PI_OFF BW9 BW12 WT"

# SINGLE
for method in $METHODS; do
    echo "Single ${method} >>>>>>>>>>>>>>>>>>"
    cmake . -DPR_TRIANGLE_INTERSECTION_METHOD=${method} -DPR_COLLIDER_FORCE_SINGLE_TRACE=ON && ninja -j4 pearray
    if [ $? -eq 0 ]; then
        $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/${method}_S -e bin/pearray $@
    else
        echo "Compile failed"
        exit 1
    fi
done

# PACKAGE
for method in $METHODS; do
    echo "Package ${method} >>>>>>>>>>>>>>>>>"
    cmake . -DPR_TRIANGLE_INTERSECTION_METHOD=${method} -DPR_COLLIDER_FORCE_SINGLE_TRACE=OFF && ninja -j4 pearray
    if [ $? -eq 0 ]; then
        $THIS_DIR/benchmark.py -o $CURR_DIR/benchmarks/${method}_P -e bin/pearray $@
    else
        echo "Compile failed"
        exit 1
    fi
done

cd $CURR_DIR