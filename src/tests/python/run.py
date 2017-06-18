import argparse
import sys
import os
import importlib


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("BINARY")
    parser.add_argument("MODULE")
    args = parser.parse_args()

    bindir = os.path.dirname(os.path.realpath(args.BINARY))
    modname = os.path.basename(os.path.realpath(args.BINARY))

    sys.path.insert(0, bindir)
    pr = importlib.import_module(modname.split('.')[0])

    if(not pr):
        print("Couldn't load binary '%s'" % args.BINARY)
        sys.exit(-1)

    test = importlib.import_module(args.MODULE)

    if(not test):
        print("Couldn't load test '%s'" % args.MODULE)
        sys.exit(-1)

    if(not hasattr(test, 'runTest')):
        print("Invalid test '%s'" % args.MODULE)
        sys.exit(-1)

    if(not callable(test.runTest)):
        print("Invalid test '%s': Not a callable" % args.MODULE)
        sys.exit(-1)

    test.runTest(pr)
