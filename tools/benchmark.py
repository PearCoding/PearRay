#!/usr/bin/python3
# -*- coding: utf-8 -*-

import subprocess
import sys
import os
import time
import argparse
from timeit import default_timer as timer


def beep():
    notes = [(0.25,440), (0.25,480), (0.25,440), (0.25,480), (0.25,440), (0.25,480), (0.25,440), (0.5,520)]

    try:
        import winsound
        for i in range(len(notes)):
            winsound.Beep(notes[i][1], notes[i][0]*1000)
    except:
        for i in range(len(notes)):
            os.system('play -nq -t alsa synth {} sine {}'.format(notes[i][0], notes[i][1]))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("-e", "--executable", help="PearRay executable", default="pearray")
    parser.add_argument("-c", "--count", help="Number of calls", metavar='N', type=int, default=5)
    parser.add_argument("-p", "--project", help="Project file", required=True)
    parser.add_argument("-o", "--output", help="Output directory", default="./benchmark")
    parser.add_argument("-t", "--threads", help="Number of threads", metavar='N', type=int, default=0)
    parser.add_argument("-q", "--quite", help="Be quite", action="store_true")
    parser.add_argument("--superquite", help="Be super quite", action="store_true")
    parser.add_argument("-b", "--beep", help="Beep when done", action="store_true")

    args = parser.parse_args()

    if not os.path.exists(args.output):
        os.makedirs(args.output)

    PR_DEF_ARGS=["-P", "-t", str(args.threads)]
    if args.quite:
        PR_DEF_ARGS.append("-q")
    else:
        PR_DEF_ARGS.append("-p") # Show progress

    res_file = open(args.output + "/result.txt", "w")
    full_time = 0
    for i in range(args.count):
        if not args.superquite:
            print(">> Task %i/%i" % (i+1, args.count))

        output_dir = args.output + "/%i" % (i+1)
        cmd = [args.executable, *PR_DEF_ARGS, "-i", args.project, "-o", output_dir]

        start = timer()
        ret_code = subprocess.call(cmd)
        if ret_code != 0:
            print("Error calling PearRay executable.")
            sys.exit(-1)
        end = timer()
        diff = end-start
        full_time += diff
        if not args.superquite:
            print("%f s" % diff)
        res_file.write("Task %i = %f s\n" % (i+1, diff))

    # Calculate average
    avg = full_time/args.count
    if not args.superquite:
        print("Full: %f s | Avg: %f s" % (full_time, avg))
    res_file.write("Full = %f s\n" % full_time)
    res_file.write("Avg = %f s\n" % avg)

    if args.beep:
        beep()
