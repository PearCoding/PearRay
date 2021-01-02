from . import colexport


def getRefl(operator, op, name, default):
    if name in op.parameters:
        return colexport.unpackRefl(operator, op.parameters[name])
    else:
        return colexport.unpackRefl(operator, default)

def getScal(operator, op, name, default):
    val = op.parameters.get(name, default)
    if isinstance(val, str):
        return "(texture '%s')" % val
    elif isinstance(val, list):
        return str(sum(val) / len(val))  # TODO:
    else:
        return str(val)

WRAP_MAP = {'repeat': 'periodic', 'black': 'black', 'clamp': 'clamp'}


def export(operator, op):
    textype = op.parameters['']
    if textype == "constant": # TODO: Spectral??
        operator.w.write("(node")
        operator.w.goIn()
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":type 'constant'")
        operator.w.write("%f" % float(op.parameters["value"]))
        operator.w.goOut()
        operator.w.write(")")
    elif textype == "scale":
        operator.w.write("(node")
        operator.w.goIn()
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":type 'smul'")
        operator.w.write(getRefl(operator, op, "tex1", 1))
        operator.w.write(getRefl(operator, op, "tex2", 1))
        operator.w.goOut()
        operator.w.write(")")
    elif textype == "mix":
        operator.w.write("(node")
        operator.w.goIn()
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":type 'sblend'")
        operator.w.write(getScal(operator, op, "amount", 0.5))
        operator.w.write(getRefl(operator, op, "tex1", 0))
        operator.w.write(getRefl(operator, op, "tex2", 1))
        operator.w.goOut()
        operator.w.write(")")
    elif textype == "imagemap":
        has_scale = "scale" in op.parameters
        tex_name = "%s-tex" % op.operand if has_scale else op.operand

        filename = op.parameters["filename"]
        inc_file = operator.resolveInclude(op.filename, filename)

        mapper = op.parameters['wrap'] if 'wrap' in op.parameters else "repeat"
        if not mapper in WRAP_MAP:
            print("Unknown wrap method '%s'" % mapper)

        operator.w.write("(texture")
        operator.w.goIn()
        operator.w.write(":name '%s'" % tex_name)
        operator.w.write(":type 'color'")
        operator.w.write(":file '%s'" % inc_file)
        operator.w.write(":wrap '%s'" % WRAP_MAP.get(mapper, WRAP_MAP['repeat']))
        operator.w.goOut()
        operator.w.write(")")

        if has_scale:
            operator.w.write("(node")
            operator.w.goIn()
            operator.w.write(":name '%s'" % op.operand)
            operator.w.write(":type 'smul'")
            operator.w.write("%f, '%s'" %
                         (float(op.parameters["scale"]), tex_name))
            operator.w.goOut()
            operator.w.write(")")
    elif textype == "checkerboard":
        operator.w.write("(node")
        operator.w.goIn()
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":type 'checkerboard'")
        operator.w.write(getRefl(operator, op, "tex1", 0))
        operator.w.write(getRefl(operator, op, "tex2", 1))
        if "uscale" in op.parameters:
            operator.w.write(getScal(operator, op, "uscale", 1))
            operator.w.write(getScal(operator, op, "vscale", 1))
        operator.w.goOut()
        operator.w.write(")")
    else:
        print("Does not support '%s' texture types" % textype)
