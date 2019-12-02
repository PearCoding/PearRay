#!/usr/bin/python
# -*- coding: utf-8 -*-

# https://github.com/mmp/pbrt-v3/
# Converts a subset of the PBRT project files into PearRay's own file format
# Not everything has a meaningful counterpart, some information is skipped

# Use:
# pr_pbrt2prc INPUT OUTPUT

import sys
import pbrt
import datetime


# Main
if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("Not enough arguments given. Need an input and output file")
        exit()

    input = sys.argv[1]
    output = sys.argv[2]

    operations = None
    with open(input, "r") as file:
        parser = pbrt.Parser()
        operations = parser.parse(input, file.read())

    if operations is None:
        print("Nothing to produce")
        sys.exit(0)

    with open(output, "w") as file:
        writer = pbrt.Writer(file)
        writer.write("; generated by pbrt2prc at %s\n" % datetime.datetime.now())

        operator = pbrt.Operator(writer)
        operator.apply(operations)

