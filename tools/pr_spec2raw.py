#!/usr/bin/python
# -*- coding: utf-8 -*-

# Use:
# pr_spec2xyz INPUT OUTPUT

import sys
import os
import struct
import pypearray as pr

# Main
if __name__=='__main__':
    
    if len(sys.argv) != 3:
        print("Not enough arguments given. Need an input and output file")
        exit()

    input = sys.argv[1]
    output = sys.argv[2]

    inputFile = pr.SpectralFile.open(input)

    with open(output, 'wb') as f:
        for i in range(1,inputFile.height):
            for j in range(1,inputFile.width):
                spec = inputFile[i-1,j-1]
                for k in range(1,pr.Spectrum.SAMPLING_COUNT):
                    f.write(struct.pack("!f", spec[k-1]))
