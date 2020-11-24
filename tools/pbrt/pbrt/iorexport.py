import numpy as np


# TODO
_IORMap = {"bk7": "(lookup_index 'bk7')",
           "glass": "(lookup_index 'bk7')",
           "water": "(lookup_index 'water')",
           "diamond": "(lookup_index 'diamond')"}


def _mapIOR(name):
    if name in _IORMap:
        return _IORMap[name]
    else:
        return None


def unpackIOR(operator, col):
    if isinstance(col, str):
        mapped = _mapIOR(col)
        if mapped is not None:
            return mapped
        else:
            return "(texture '%s')" % col
    elif isinstance(col, float):
        return "%f" % (col)
    elif len(col) == 3:
        # TODO
        # return "(refl %f %f %f)" % (col[0], col[1], col[2])
        return "%f" % ((col[0]+col[1]+col[2])/3.0)
    else:
        print("Invalid IOR given")
        return "1.55"


# TODO
def _mapK(name):
    return None


def unpackK(operator, col):
    if isinstance(col, str):
        mapped = _mapK(col)
        if mapped is not None:
            return mapped
        else:
            return "(texture '%s')" % col
    elif isinstance(col, float):
        return "%f" % (col)
    elif len(col) == 3:
        # TODO
        # return "(refl %f %f %f)" % (col[0], col[1], col[2])
        return "%f" % ((col[0]+col[1]+col[2])/3.0)
    else:
        print("Invalid K factor given")
        return "0.05"
