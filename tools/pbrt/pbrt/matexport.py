from . import colexport
from . import iorexport


def getRefl(operator, op, name, default):
    if name in op.parameters:
        return colexport.unpackRefl(operator, op.parameters[name])
    else:
        return colexport.unpackRefl(operator, default)


def getIOR(operator, op, name, default):
    if name in op.parameters:
        return iorexport.unpackIOR(operator, op.parameters[name])
    else:
        return iorexport.unpackIOR(operator, default)


def getK(operator, op, name, default):
    if name in op.parameters:
        return iorexport.unpackK(operator, op.parameters[name])
    else:
        return iorexport.unpackK(operator, default)


def getScal(operator, op, name, default):
    val = op.parameters.get(name, default)
    if isinstance(val, str):
        return "(texture '%s')" % val
    elif isinstance(val, list):
        return str(sum(val) / len(val))  # TODO:
    else:
        return str(val)


def getRoughness(operator, op):
    if "uroughness" in op.parameters and "vroughness" in op.parameters:
        roughness_x = getScal(operator, op, 'uroughness', 0)
        roughness_y = getScal(operator, op, 'vroughness', 0)
    elif "uroughness" in op.parameters:
        roughness_x = getScal(operator, op, 'uroughness', 0)
        roughness_y = roughness_x
    elif "roughness" in op.parameters:
        roughness_x = getScal(operator, op, 'roughness', 0)
        roughness_y = roughness_x
    else:
        return None

    return (roughness_x, roughness_y)


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
        operator.w.write(":name '%s'" % name)
        operator.w.write(":albedo %s" % color)
        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "glass":
        color = getRefl(operator, op, 'Ks', [1, 1, 1])
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'glass'")
        operator.w.write(":name '%s'" % name)
        operator.w.write(":specular %s" % color)
        operator.w.write(":index %s" % getIOR(operator, op, 'eta', 'bk7'))

        roughness = getRoughness(operator, op)
        if roughness is not None:
            if roughness[0] != roughness[1]:
                operator.w.write(":roughness_x %s" % roughness[0])
                operator.w.write(":roughness_y %s" % roughness[1])
            else:
                operator.w.write(":roughness %s" % roughness[0])

        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "metal":
        eta = getIOR(operator, op, 'eta', 'copper')
        k = getK(operator, op, 'k', 'copper')
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'conductor'")
        operator.w.write(":name '%s'" % name)
        operator.w.write(":eta %s" % eta)
        operator.w.write(":k %s" % k)

        roughness = getRoughness(operator, op)
        if roughness is not None:
            if roughness[0] != roughness[1]:
                operator.w.write(":roughness_x %s" % roughness[0])
                operator.w.write(":roughness_y %s" % roughness[1])
            else:
                operator.w.write(":roughness %s" % roughness[0])

        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "mirror":
        color = getRefl(operator, op, 'Kr', [1, 1, 1])
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'mirror'")
        operator.w.write(":name '%s'" % name)
        operator.w.write(":specular %s" % color)
        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "substrate":  # TODO: Not yet implemented
        print("WARNING: 'substrate' material not yet fully implemented!")
        colorDiff = getRefl(operator, op, 'Kd', [0.5, 0.5, 0.5])

        roughness = ""
        aniso = 1
        if "uroughness" in op.parameters and "vroughness" in op.parameters:
            aniso = float(op.parameters["uroughness"]) / \
                float(op.parameters["vroughness"])
            roughness = getScal(operator, op, 'uroughness', 0.5)
        else:
            roughness = getScal(operator, op, 'roughness', 0.5)

        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'principled'")  # TODO:
        operator.w.write(":name '%s'" % name)
        operator.w.write(":base %s" % colorDiff)
        operator.w.write(":specular %s" % getScal(operator, op, 'Ks', 0.5))
        operator.w.write(":roughness %s" % roughness)
        operator.w.write(":anisotropy %f" % aniso)
        operator.w.goOut()
        operator.w.write(")")
    elif mat_type == "disney":
        color = getRefl(operator, op, 'color', [1, 1, 1])
        eta = getRefl(operator, op, 'eta', 1)
        spec = ((eta - 1) / (eta + 1))**2
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'principled'")
        operator.w.write(":name '%s'" % name)
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
    elif mat_type == "uber":
        print("WARNING: 'uber' material will be approximated!")
        diff = getRefl(operator, op, 'Kd', [0.25, 0.25, 0.25])
        spec = getRefl(operator, op, 'Ks', [0.25, 0.25, 0.25])
        refl = getRefl(operator, op, 'Kr', [0, 0, 0])
        tran = getRefl(operator, op, 'Kt', [0, 0, 0])
        opacity = getScal(operator, op, 'opacity', 1)
        eta = getIOR(operator, op, 'eta', 1.55)

        hasOpacity = "opacity" in op.parameters
        hasGlass = "Kr" in op.parameters or "Kt" in op.parameters

        # Opacity material
        if hasOpacity:
            operator.w.write("(material")
            operator.w.goIn()
            operator.w.write(":type 'null'")
            operator.w.write(":name '%s-opacity'" % name)
            operator.w.goOut()
            operator.w.write(")")

        # Glass material
        if hasGlass:
            operator.w.write("(material")
            operator.w.goIn()
            operator.w.write(":type 'glass'")
            operator.w.write(":name '%s-glass'" % name)
            operator.w.write(":specular %s" % spec)
            operator.w.write(":transmission (smul %s %s)" % (tran, spec))
            operator.w.write(":index %s" % eta)
            operator.w.goOut()
            operator.w.write(")")

        # Diffuse material
        diffName = "%s-diff" % name if hasOpacity or hasGlass else name

        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'diffuse'")
        operator.w.write(":name '%s'" % diffName)
        operator.w.write(":albedo %s" % diff)
        operator.w.goOut()
        operator.w.write(")")

        # Combine glass with diffuse
        if hasGlass:
            in1 = "%s-mix" % name
            operator.w.write("(material")
            operator.w.goIn()
            operator.w.write(":type 'blend'")
            operator.w.write(":name '%s'" % in1)
            operator.w.write(":material1 '%s'" % diffName)
            operator.w.write(":material2 '%s-glass'" % name)
            operator.w.write(":factor %s" % refl)
            operator.w.goOut()
            operator.w.write(")")
        else:
            in1 = diffName

        # Combine opacity with diffuse or mix of glass and diffuse
        if hasOpacity:
            operator.w.write("(material")
            operator.w.goIn()
            operator.w.write(":type 'blend'")
            operator.w.write(":name '%s'" % name)
            operator.w.write(":material1 '%s-opacity'" % name)
            operator.w.write(":material2 '%s'" % in1)
            operator.w.write(":factor %s" % opacity)
            operator.w.goOut()
            operator.w.write(")")

    elif mat_type == "mix":
        mat1 = op.parameters.get("namedmaterial1", "")
        mat2 = op.parameters.get("namedmaterial2", "")
        fac = getScal(operator, op, 'amount', 0.5)
        operator.w.write("(material")
        operator.w.goIn()
        operator.w.write(":type 'mix'")
        operator.w.write(":name '%s'" % name)
        operator.w.write(":material1 '%s'" % mat1)
        operator.w.write(":material2 '%s'" % mat2)
        operator.w.write(":factor %s" % fac)
        operator.w.goOut()
        operator.w.write(")")
    else:
        print("ERROR: No support of materials of type %s for %s available" %
              (mat_type, name))

    operator.matCount += 1
    return name
