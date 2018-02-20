import argparse
import sys
import os
import importlib

#
# Test runner
# Will get binary to pypearray from the cmake/ctest build system
# and the module to test
#

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("BINARY")
    parser.add_argument("MODULE")
    args = parser.parse_args()

    bindir = os.path.dirname(os.path.realpath(args.BINARY))
    modname = os.path.basename(os.path.realpath(args.BINARY))

    sys.path.insert(0, bindir)

    print("Loading module %s" % (modname))
    pr = importlib.import_module(modname.split('.')[0])

    print("Starting test: %s" % (args.MODULE))

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

    success = test.runTest(pr)

    if not success:
        sys.exit(-2)
else:
    print("run.py is being imported into another module. This is not allowed!")
    sys.exit(-3)