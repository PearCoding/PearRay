#!/usr/bin/python3
# -*- coding: utf-8 -*-

# https://github.com/mmp/pbrt-v3/
# Converts a subset of the PBRT project files into PearRay's own file format
# Not everything has a meaningful counterpart, some information is skipped

# Use:
# pr_pbrt2prc INPUT OUTPUT

import sys
import os
import pbrt
import argparse


def parseArgs():
    parser = argparse.ArgumentParser(
        description="Converts a subset of the PBRT project files into PearRay's own file format.")

    parser.add_argument('input', help="Input file")
    parser.add_argument('-o', '--output', default=os.getcwd(), help="Output directory")
    parser.add_argument('--skipMesh', action='store_true', default=False, help="Skip Mesh Conversion")
    parser.add_argument('--skipCopies', action='store_true', default=False, help="Skip Instanced Generation")
    parser.add_argument('--skipLight', action='store_true', default=False, help="Skip Light Generation")
    parser.add_argument('--skipMat', action='store_true', default=False, help="Skip Material Generation")
    parser.add_argument('--skipTex', action='store_true', default=False, help="Skip Texture Generation")
    parser.add_argument('--skipCamera', action='store_true', default=False, help="Skip Camera Generation")
    parser.add_argument('--skipReg', action='store_true', default=False, help="Skip Registry Generation")
    parser.add_argument('--singleFile', action='store_true', default=False, help="Produce one single file")

    return parser.parse_args()

# Main
if __name__ == '__main__':
    args = parseArgs()

    operations = None
    with open(args.input, "r") as file:
        parser = pbrt.Parser()
        operations = parser.parse(args.input, file.read())

    if operations is None:
        print("Nothing to produce")
        sys.exit(0)

    main_file = os.path.join(args.output, os.path.splitext(os.path.basename(args.input))[0]+'.prc')
    os.makedirs(args.output, exist_ok=True)

    #print(operations)
    with open(main_file, "w") as file:
        writer = pbrt.Writer(file)
        writer.write("(scene")
        writer.goIn()

        operator = pbrt.Operator(writer, args, os.path.dirname(os.path.abspath(args.input)))
        operator.apply(operations)

        writer.goOut()
        writer.write(")")

