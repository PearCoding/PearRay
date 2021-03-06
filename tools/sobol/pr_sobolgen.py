#!/usr/bin/python3
# -*- coding: utf-8 -*-

# Based on http://web.maths.unsw.edu.au/~fkuo/sobol/index.html
# To understand how to use it, use
#   pr_sobolgen --help

import argparse


# Get index from the right of the first zero bit
def irfz(n):
    counter = 1
    val = n
    while val & 1:
        val >>= 1
        counter += 1
    return counter


def loadFile(max_dims, file):
    if max_dims < 2:
        return

    next(file)  # Skip first line

    # Dimension 1 is handled as a special case!
    S = []
    A = []
    M = []

    dim = 2
    for line in file:
        arr = [int(x) for x in line.split()]
        if arr[0] != dim:
            print("Invalid file given!")
            return

        S.append(arr[1])
        A.append(arr[2])
        if len(arr[3:]) != arr[1]:
            print("S and size of M does not match!")
            return

        M.append([1] + arr[3:])

        dim += 1
        if dim > max_dims:
            break

    return S, A, M


def generateV(dim, max_bits, S, A, M):
    if dim == 1:
        return [1 << (max_bits-i-1) for i in range(max_bits)]
    else:
        s = S[dim-2]
        a = A[dim-2]
        m = M[dim-2]
        V = [0] * (max_bits)
        for i in range(s):
            V[i] = m[i+1] << (max_bits-i-1)
        for i in range(s, max_bits):
            V[i] = V[i-s] ^ (V[i-s] >> s)
            for k in range(1,s):
                V[i] ^= (((a >> (s-1-k)) & 1) * V[i-k])
        return V


def generateCode(file, dim, max_bits, V):
    file.write("#pragma once\n")
    file.write("// Generated by py_sobolgen.py for the PearRay project\n")
    file.write("constexpr size_t MAX_DIM=%i;\n" % dim)
    file.write("constexpr size_t MAX_BITS=%i;\n" % max_bits)
    if max_bits == 64:
        file.write("#define _SOBOL_64\n")
        file.write("typedef uint64_t sobol_entry_t;\n")
    else:
        file.write("#define _SOBOL_32\n")
        file.write("typedef uint32_t sobol_entry_t;\n")

    file.write("static const sobol_entry_t sobol_V[MAX_DIM][MAX_BITS]={\n")
    file.write(",\n".join(["{%s}" % (",".join(str(v)+"u" for v in V[i])) for i in range(dim)]))
    file.write("\n};\n")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="py_sobolgen")
    parser.add_argument(
        "max_dim", help="Maximum dimension to create data", type=int)
    parser.add_argument(
        "input", help="Direction number files obtained at the website: https://web.maths.unsw.edu.au/~fkuo/sobol/", type=argparse.FileType('r'))
    parser.add_argument("-g", "--generate", help="Generate code for C/C++",
                        metavar="OUTPUT", type=argparse.FileType('w'))
    parser.add_argument(
        "-p", "--print", help="Print generated matrix", action="store_true")
    parser.add_argument("-v", "--verbose",
                        help="Print loaded content", action="store_true")
    parser.add_argument(
        "-r", "--run", help="Generate random numbers", type=int, metavar="N")
    parser.add_argument(
        "-x64", help="Generate for 64bit instead of 32bit", action="store_true")

    args = parser.parse_args()

    max_dims = args.max_dim

    if max_dims < 1:
        print("Invalid dimension given")
        exit()

    input = args.input

    S, A, M = loadFile(max_dims, input)
    if not S:
        print("Error while loading file")
        exit()

    if args.verbose:
        print(S)
        print(A)
        print(M)

    if len(S) + 1 != max_dims:
        print("Requested %i dimensions but only %i available" %
              (max_Dims, len(S) + 1))
        max_dims = len(S) + 1

    max_bits = 64 if args.x64 else 32

    V = []
    for i in range(max_dims):
        V.append(generateV(i+1, max_bits, S, A, M))

    if args.print:
        print(V)

    if args.generate:
        generateCode(args.generate, max_dims, max_bits, V)

    if args.run and args.run > 0:
        MAX_VAL = 2**max_bits
        val = [0]*max_dims
        print(val)
        for i in range(1, args.run):
            cin = irfz(i-1)
            for k in range(max_dims):
                val[k] ^= V[k][cin-1]
            print([k / float(MAX_VAL) for k in val])
