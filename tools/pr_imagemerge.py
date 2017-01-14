#!/usr/bin/python
# -*- coding: utf-8 -*-

# Use:
# pr_imagemerge PEARRAY_OUTPUT_DIR

# PEARRAY_OUTPUT_DIR is the directory specified with -o in pearray

TOOL="oiiotool"

import sys
import subprocess
import os
import re;

def run_tool(input1, input2, output):
    args = [input1,
        input2, 
        "--add",
        "-o",
        output
    ]

    try:
        _process = subprocess.call([TOOL] + args)
    except OSError:
        print("Could not execute '%s'" % TOOL)

# Main
if __name__=='__main__':
    
    if len(sys.argv) != 2:
        print("Not enough arguments given. Need a pearray output directory")
        exit()

    dir = sys.argv[1]
    resultsDirs = [f for f in os.listdir(dir) if os.path.isdir(os.path.join(dir, f))]

    if not 'results' in resultsDirs:
        print("Not a valid pearray output directory")
        exit()

    resultsDirs = filter(re.compile('results_[0-9]+').match, resultsDirs)
    resultsDirs.append('results')# results_0
    resultsDirs = [os.path.abspath(os.path.join(dir, f)) for f in resultsDirs]

    if len(resultsDirs) == 1:
        exit()

    inputFiles = [f for f in os.listdir(os.path.join(dir, 'results')) if f.endswith('.exr')]

    for i,f in enumerate(inputFiles):
        print("Computing output %s [%i/%i]" % (f, i+1, len(inputFiles)))

        outputFile = os.path.join(dir, 'merged_' + f)

        print("Merging %i/%i..." % (2, len(resultsDirs)))
        run_tool(os.path.join(resultsDirs[0], f), os.path.join(resultsDirs[1], f), outputFile)
        for j,o in enumerate(resultsDirs[2:]):
            print("Merging %i/%i..." % (j+3, len(resultsDirs)))
            run_tool(outputFile, os.path.join(o, f), outputFile)

    print("Finished")