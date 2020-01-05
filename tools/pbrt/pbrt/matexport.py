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
    else:
        return str(val)


def export(operator, op, isNamed):
    if isNamed:
        mat_type = op.parameters.get('type', 'NONE')
        name = op.operand
    else:
        mat_type = op.operand
        name = "material_%i" % operator.matCount

    if mat_type == "matte":
        color = getRefl(operator, op, 'Kd', [1, 1, 1])
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'diffuse'")
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":albedo %s" % color)
        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "glass":
        color = getRefl(operator, op, 'Ks', [1, 1, 1])
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'glass'")
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":specular %s" % color)
        operator.w.write(":index %s" % getScal(operator, op, 'eta', 1.55))
        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "disney":
        color = getRefl(operator, op, 'color', [1, 1, 1])
        eta = getScal(operator, op, 'eta', 1)
        spec = ((eta - 1) / (eta + 1))**2
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'principled'")
        operator.w.write(":name '%s'" % op.operand)
        operator.w.write(":base %s" % color)
        operator.w.write(":roughness %s" %
                         getScal(operator, op, 'roughness', 0.5))
        operator.w.write(":specular %s" % spec)
        operator.w.write(":specular_tint %s" %
                         getScal(operator, op, 'speculartint', 0))
        operator.w.write(":metallic %s" % getScal(operator, op, 'metallic', 0))
        operator.w.write(":clearcoat %s" %
                         getScal(operator, op, 'clearcoat', 0))
        operator.w.write(":clearcoat_gloss %s" %
                         getScal(operator, op, 'clearcoatgloss', 0))
        operator.w.write(":anisotropic %s" %
                         getScal(operator, op, 'anisotropic', 0))
        operator.w.write(":sheen %s" % getScal(operator, op, 'sheen', 0))
        operator.w.write(":sheen_tint %s" %
                         getScal(operator, op, 'sheentint', 0))
        # TODO: 'spectrans', 'scatterdistance' ?
        operator.w.goOut()
        operator.w.write(")")
    else:
        print("ERROR: No support of materials of type %s for %s available" %
              (mat_type, name))

    operator.matCount += 1
    return name
