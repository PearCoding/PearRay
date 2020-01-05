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
    return unpackSpec(operator, col, 'refl')


def unpackIllum(operator, col):
    return unpackSpec(operator, np.multiply(operator.options.lightFactor, col), 'illum')
