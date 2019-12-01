#!/usr/bin/python
# -*- coding: utf-8 -*-

# https://github.com/mmp/pbrt-v3/
# Converts a subset of the PBRT project files into PearRay's own file format
# Not everything has a meaningful counterpart, some information is skipped

# Use:
# pr_pbrt2prc INPUT OUTPUT

import sys
import pbrt


# Main
if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Not enough arguments given. Need an input and output file")
        exit()

    input = sys.argv[1]
    output = sys.argv[2]

    parser = pbrt.Parser()
    with open(input, "r") as file:
        operations = parser.parse(file.read())

    print(str(operations))

    # TODO: Operations ->
