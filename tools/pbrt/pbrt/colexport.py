import numpy as np


def unpackSpec(operator, col, spec_type):
    if isinstance(col, str):
        return "(texture '%s')" % col

    name = "spec_%i" % operator.specCount
    operator.specCount += 1

    operator.w.write("(spectrum")
    operator.w.goIn()
    operator.w.write(":name '%s'" % name)
    if not np.isscalar(col):
        operator.w.write(":data (%s %f %f %f)" %
                         (spec_type, col[0], col[1], col[2]))
    else:
        operator.w.write(":data (%s %f %f %f)" % (spec_type, col, col, col))
    operator.w.goOut()
    operator.w.write(")")
    return "'%s'" % name


def unpackRefl(operator, col):
    if isinstance(col, str):
        return "(texture '%s')" % col

    return "(refl %f %f %f)" % (col[0], col[1], col[2])


def unpackIllum(operator, col):
    if isinstance(col, str):
        return "(texture '%s')" % col

    if operator.options.lightFactor == 1.0:
        return "(illum %f %f %f)" % (col[0], col[1], col[2])
    else:
        return "(smul (illum %f %f %f) %f)" % (col[0], col[1], col[2], operator.options.lightFactor)
