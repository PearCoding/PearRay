#!/usr/bin/python3
# -*- coding: utf-8 -*-

# https://github.com/mmp/pbrt-v3/
# Converts a subset of the PBRT project files into PearRay's own file format
# Not everything has a meaningful counterpart, some information is skipped

# Use:
# pbrt2prc INPUT OUTPUT

import os
import pbrt
import argparse


def parseArgs():
    parser = argparse.ArgumentParser(
        description="Converts a subset of the PBRT project files into PearRay's own file format.")

    parser.add_argument('input', help="Input file")
    parser.add_argument('-o', '--output', default=os.getcwd(),
                        help="Output directory")
    parser.add_argument('-q', '--quite', action='store_true',
                        default=False, help="Do not print any informative messages")
    parser.add_argument('--include_offset', default='./',
                        help="Include offset")
    parser.add_argument('--skipMesh', action='store_true',
                        default=False, help="Skip Mesh Conversion")
    parser.add_argument('--skipCurve', action='store_true',
                        default=False, help="Skip Curve Generation")
    parser.add_argument('--skipPrim', action='store_true',
                        default=False, help="Skip Primitive Generation")
    parser.add_argument('--skipInstance', action='store_true',
                        default=False, help="Skip Instanced Generation")
    parser.add_argument('--skipLight', action='store_true',
                        default=False, help="Skip Light Generation")
    parser.add_argument('--skipMat', action='store_true',
                        default=False, help="Skip Material Generation")
    parser.add_argument('--skipTex', action='store_true',
                        default=False, help="Skip Texture Generation")
    parser.add_argument('--skipCamera', action='store_true',
                        default=False, help="Skip Camera Generation")
    parser.add_argument('--skipWorld', action='store_true',
                        default=False, help="Skip World Generation")
    parser.add_argument('--skipRepetitiveIncludes', action='store_true',
                        default=False, help="Skip repetitive include statements")
    parser.add_argument('--singleFile', action='store_true',
                        default=False, help="Produce one single file")

    return parser.parse_args()


# Main
if __name__ == '__main__':
    args = parseArgs()

    main_file = os.path.join(args.output,
                             os.path.splitext(os.path.basename(args.input))[0]+'.prc')
    os.makedirs(args.output, exist_ok=True)

    with open(main_file, "w") as output_file:
        with open(args.input, "r") as input_file:
            writer = pbrt.Writer(output_file)
            if not args.skipWorld:
                writer.write("(scene")
                writer.goIn()

            operator = pbrt.Operator(
                writer, args, os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(args.input)), args.include_offset)))
            for op in pbrt.Parser.parse(args.input, input_file):
                # print(op)
                operator.apply(op)

            if not args.skipWorld:
                writer.goOut()
                writer.write(")")
