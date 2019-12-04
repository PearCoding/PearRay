import os
import numpy as np
import math

from .parser import Parser


class Operator:
    def __init__(self, writer):
        self.writer = writer
        self.w = self.writer
        self.coords = {}
        self.transformStack = [np.identity(4)]
        self.currentObject = ""
        self.currentMaterial = ""
        self.objectCount = 0

    def apply(self, operations):
        # print(str(operations))
        for op in operations:
            try:
                method = getattr(self, "op_" + op.action)
            except AttributeError:
                print("Unknown Action %s" % op.action)
            else:
                method(op)

    @staticmethod
    def mat2str(mat):
        return "[" + ",".join(",".join(str(f) for f in row) for row in mat) + "]"

    @staticmethod
    def resolve_filename(base, filename):
        file_dir = os.path.dirname(base)
        return filename if os.path.isabs(
            filename) else os.path.join(file_dir, filename)

    def op_Include(self, op):
        inc_file = Operator.resolve_filename(op.filename, op.operand)

        operations = None
        try:
            with open(inc_file, "r") as file:
                parser = Parser()
                operations = parser.parse(inc_file, file.read())
        except IOError as e:
            print(e)

        if operations is not None:
            self.apply(operations)

    def op_Identity(self, op):
        self.transformStack[-1] = np.identity(4)

    def applyTransform(self, mat):
        self.transformStack[-1] = np.dot(self.transformStack[-1], mat)
        print(self.transformStack[-1])

    def op_Translate(self, op):
        self.applyTransform(np.array([[1, 0, 0, op.operand[0]],
                                             [0, 1, 0, op.operand[1]],
                                             [0, 0, 1, op.operand[2]],
                                             [0, 0, 0, 1]]))

    def op_Rotate(self, op):
        co = math.cos(op.operand[0])
        si = math.sin(op.operand[0])

        x = op.operand[1]
        y = op.operand[2]
        z = op.operand[3]

        self.applyTransform(np.array([[co+x*x*(1-co), x*y*(1-co)-z*si, x*z*(1-co)+y*si, 0],
                                             [y*x*(1-co)+z*si, co+y*y*(1-co), y*z*(1-co)-x*si, 0],
                                             [z*x*(1-co)-y*si, z*y*(1-co)+x*si, co+z*z*(1-co), 0],
                                             [0, 0, 0, 1]]))

    def op_Scale(self, op):
        self.applyTransform(np.array([[op.operand[0], 0, 0, 0],
                                             [0, op.operand[1], 0, 0],
                                             [0, 0, op.operand[2], 0],
                                             [0, 0, 0, 1]]))

    def op_Transform(self, op):
        self.transformStack[-1] = np.transpose(np.array([op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]]))
        print(self.transformStack[-1])

    def op_ConcatTransform(self, op):
        self.applyTransform(np.transpose(np.array([op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]])))

    def op_CoordinateSystem(self, op):
        self.coords[op.operand[0]] = self.transformStack[-1]

    def op_CoordinateSysTransform(self, op):
        self.transformStack[-1] = self.coords[op.operand[0]]

    def op_Camera(self, op):
        self.coords['camera'] = np.linalg.inv(self.transformStack[-1])

        # Only support one camera
        self.w.write(":camera 'Camera'")
        self.w.write("(camera")
        self.w.goIn()
        self.w.write(":name 'Camera'")
        self.w.write(":type 'standard'")
        self.w.write(":transform %s" % Operator.mat2str(self.coords['camera']))
        self.w.goOut()
        self.w.write(")")

    def op_WorldBegin(self, op):
        self.op_Identity(op)
        self.coords['world'] = self.transformStack[-1]

    def op_WorldEnd(self, op):
        while len(self.transformStack) != 1:
            self.transformStack.pop()

    def op_AttributeBegin(self, op):
        self.transformStack.append(self.transformStack[-1])

    def op_AttributeEnd(self, op):
        if len(self.transformStack) == 1:
            print("Can not pop transform stack anymore!")
        else:
            self.transformStack.pop()

    def op_TransformBegin(self, op):
        self.op_AttributeBegin(op)

    def op_TransformEnd(self, op):
        self.op_AttributeEnd(op)

    def op_ObjectBegin(self, op):
        if self.currentObject != "":
            print("Already inside object definition")

        self.currentObject = op.operand[0]
        self.op_AttributeBegin(op)

    def op_ObjectEnd(self, op):
        if self.currentObject == "":
            print("Already outside object definition")

        self.currentObject = ""
        self.op_AttributeEnd(op)

    # TODO: Add simple copy instance support to PearRay!
    def op_ObjectInstance(self, op):
        if self.currentObject != "":
            print("Already inside object definition")

    def op_Integrator(self, op):
        self.w.write("(registry '/renderer/common/type' 'direct')")# Ignore type for now
        self.w.write("(registry '/renderer/common/max_ray_depth %i)" % op.parameters['integer maxdepth'])

    def writeSampler(self, sampler, pixel_count):
        self.w.write("(registry '/renderer/common/sampler/aa/type' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/sampler/aa/count' %i)" % pixel_count)
        self.w.write("(registry '/renderer/common/sampler/lens/type' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/sampler/lens/count' 1)")
        self.w.write("(registry '/renderer/common/sampler/time/type' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/sampler/time/count' 1)")

    def op_Sampler(self, op):
        count = op.parameters["integer pixelsamples"]
        if op.operand == "lowdiscrepancy" or op.operand == "02sequence" or op.operand == "jittered":
            self.writeSampler("multijittered", count)
        elif op.operand == "halton":
            self.writeSampler("halton", count)
        elif op.operand == "random":
            self.writeSampler("random", count)
        else:
            self.writeSampler("sobol", count)

    def op_Accelerator(self, op):
        pass # Ignoring for now

    def op_PixelFilter(self, op):
        pass # Ignoring for now

    def op_Film(self, op):
        if op.operand != "image":
            print("Only support image output!")
            return

        filename = os.path.splitext(os.path.basename(op.parameters["string filename"]))[0]
        self.w.write(":renderWidth %i" % op.parameters["integer xresolution"])
        self.w.write(":renderHeight %i" % op.parameters["integer yresolution"])
        self.w.write("(output")
        self.w.goIn()
        self.w.write(":name '%s'" % filename)
        self.w.write("(channel")
        self.w.goIn()
        self.w.write(":type 'color'")
        self.w.write(":color 'rgb'")
        self.w.write(":gamma 'srgb'")
        self.w.write(":mapper 'none'")
        self.w.goOut()
        self.w.write(")")
        self.w.write("(channel :type 'depth' )")
        self.w.write("(channel :type 'feedback' )")
        self.w.goOut()
        self.w.write(")")

    def op_MakeNamedMedium(self, op):
        pass # Ignoring for now

    def op_MediumInterface(self, op):
        pass # Ignoring for now

    def op_Texture(self, op):
        pass # TODO

    @staticmethod
    def unpackColor(col):
        if isinstance(col,list):
            return "(rgb %f %f %f)" % (col[0], col[1], col[2])
        else:
            return "(rgb %f %f %f)" % (col, col, col)

    def op_Material(self, op):
        mat_type = op.parameters['string type']
        if mat_type == "matte":
            color = Operator.unpackColor(op.parameters['rgb Kd'])
            self.w.write("(material")
            self.w.goIn()
            self.w.write(":type 'diffuse'")
            self.w.write(":name '%s'" % op.operand)
            self.w.write(":albedo %s" % color)
            self.w.goOut()
            self.w.write(")")
        else:
            print("No support of materials of type %s available" % op.operand)

        self.currentMaterial = op.operand

    def op_MakeNamedMaterial(self, op):
        self.op_Material(op)# Same in our case

    def op_NamedMaterial(self, op):
        self.currentMaterial = op.operand

    def op_LightSource(self, op):
        if op.operand == "point":
            pass # TODO
        elif op.operand == "distant":
            D = np.array(op.parameters['point to']) - np.array(op.parameters['point from'])
            D = D/np.linalg.norm(D, ord=2)
            color = Operator.unpackColor(op.parameters['rgb L'])

            self.w.write("(light")
            self.w.goIn()
            self.w.write(":type 'distant'")
            self.w.write(":direction [%f, %f, %f]" % (D[0], D[1], D[2]))
            self.w.write(":radiance %s" % color)
            self.w.goOut()
            self.w.write(")")
        else:
            print("No support of light sources of type %s available" % op.operand)

    def op_AreaLightSource(self, op):
        pass # TODO

    def op_Shape(self, op):
        if op.operand == "plymesh":
            inc_file = Operator.resolve_filename(op.filename, op.parameters['string filename'])
            self.w.write("(embed")
            self.w.goIn()
            self.w.write(":loader 'ply'")
            self.w.write(":file '%s'" % inc_file)
            self.w.write(":name 'object_%i_mesh'" % self.objectCount)
            self.w.goOut()
            self.w.write(")")

            self.w.write("(entity")
            self.w.goIn()
            self.w.write(":name 'object_%i'" % self.objectCount)
            self.w.write(":type 'mesh'")
            self.w.write(":material '%s'" % self.currentMaterial)
            self.w.write(":mesh 'object_%i_mesh'" % self.objectCount)
            self.w.write(":transform %s" % Operator.mat2str(self.transformStack[-1]))
            self.w.goOut()
            self.w.write(")")
            self.objectCount += 1
        elif op.operand == "trianglemesh":
            pass# TODO
        elif op.operand == "loopsubdiv":
            pass# TODO
        elif op.operand == "curve":
            pass# TODO
        elif op.operand == "sphere":
            self.w.write("(entity")
            self.w.goIn()
            self.w.write(":name 'object_%i'" % self.objectCount)
            self.w.write(":type 'sphere'")
            self.w.write(":material '%s'" % self.currentMaterial)
            self.w.write(":radius %f" % op.parameters['radius'])
            self.w.write(":transform %s" % Operator.mat2str(self.transformStack[-1]))
            self.w.goOut()
            self.w.write(")")
            self.objectCount += 1
        else:
            print("No support of shapes of type %s available" % op.operand)
