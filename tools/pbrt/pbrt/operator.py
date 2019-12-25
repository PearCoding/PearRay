import os
import numpy as np
import math

from .parser import Parser
from .writer import Writer


class Operator:
    def __init__(self, writer, opts, global_dir):
        self.options = opts
        self.globalDir = global_dir

        self.w = writer
        self.coords = {}
        self.transformStack = [np.identity(4)]
        self.currentObject = ""
        self.currentMaterial = ""
        self.objectCount = 0
        self.shapeCount = 0
        self.textureCount = 0
        self.objectShapeAssoc = {}

    def apply(self, operations):
        for op in operations:
            try:
                method = getattr(self, "op_" + op.action)
            except AttributeError:
                print("Unknown Action %s" % op.action)
            else:
                method(op)

    @staticmethod
    def mat2str(mat):
        return "[" + ", ".join(", ".join(str(f) for f in row) for row in mat) + "]"

    @staticmethod
    def resolveFilename(base, filename):
        file_dir = os.path.dirname(base) if os.path.isfile(base) else base
        return filename if os.path.isabs(
            filename) else os.path.join(file_dir, filename)

    def resolveInclude(self, base, filename):
        inc_file = Operator.resolveFilename(base, filename)
        if not os.path.exists(inc_file):
            inc_file = Operator.resolveFilename(self.globalDir, filename)
        return inc_file

    def op_Include(self, op):
        inc_file = self.resolveInclude(op.filename, op.operand)

        operations = None
        try:
            with open(inc_file, "r") as file:
                parser = Parser()
                operations = parser.parse(inc_file, file.read())
        except IOError as e:
            print(e)
            return

        print("Including: %s" % inc_file)
        if self.options.singleFile:
            if operations is not None:
                self.apply(operations)
        else:
            inc_file = os.path.relpath(os.path.abspath(inc_file), self.globalDir)
            output_file = os.path.join(self.options.output, os.path.splitext(inc_file)[0]+'.prc')
            os.makedirs(os.path.dirname(output_file), exist_ok=True)

            self.w.write("(include '%s')" % output_file)
            self.w.flush()

            old_writer = self.w
            with open(output_file, "w") as file:
                self.w = Writer(file)
                if operations is not None:
                    self.apply(operations)
            self.w = old_writer

    def op_Identity(self, op):
        self.transformStack[-1] = np.identity(4)

    def applyTransform(self, mat):
        self.transformStack[-1] = np.dot(self.transformStack[-1], mat)
        #print(self.transformStack[-1])

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
        #print(self.transformStack[-1])

    def op_ConcatTransform(self, op):
        self.applyTransform(np.transpose(np.array([op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]])))

    def op_LookAt(self, op):
        right = op.operand[0:3]
        up = op.operand[3:6]
        front = op.operand[6:9]
        self.applyTransform(np.array([[right[0], up[0], front[0], 0],
                                      [right[1], up[1], front[1], 0],
                                      [right[2], up[2], front[2], 0],
                                      [0, 0, 0, 1]]))

    def op_CoordinateSystem(self, op):
        self.coords[op.operand[0]] = self.transformStack[-1]

    def op_CoordinateSysTransform(self, op):
        self.transformStack[-1] = self.coords[op.operand[0]]

    def op_Camera(self, op):
        self.coords['camera'] = np.linalg.inv(self.transformStack[-1])

        if self.options.skipCamera:
            return

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
        self.objectShapeAssoc[self.currentObject] = []
        self.op_AttributeBegin(op)

    def op_ObjectEnd(self, op):
        if self.currentObject == "":
            print("Already outside object definition")

        self.currentObject = ""
        self.op_AttributeEnd(op)

    def op_ObjectInstance(self, op):
        if self.currentObject != "":
            print("Already inside object definition")

        if self.options.skipCopies:
            return

        if not op.operand in self.objectShapeAssoc:
            print("Unknown object %s" % op.operand)
            return

        assoc = self.objectShapeAssoc[op.operand]

        if assoc[0] == 'mesh':
            self.w.write("(entity")
            self.w.goIn()
            self.w.write(":name 'object_%i'" % self.objectCount)
            self.w.write(":type 'mesh'")
            self.w.write(":materials ['%s']" % assoc[2])
            self.w.write(":mesh '%s'" % assoc[1])
            self.w.write(":transform %s" % Operator.mat2str(self.transformStack[-1]))
            self.w.goOut()
            self.w.write(")")
        elif assoc[0] == 'sphere':
            self.w.write("(entity")
            self.w.goIn()
            self.w.write(":name 'object_%i'" % self.objectCount)
            self.w.write(":type 'sphere'")
            self.w.write(":material '%s'" % assoc[2])
            self.w.write(":radius %f" % assoc[1])
            self.w.write(":transform %s" % Operator.mat2str(self.transformStack[-1]))
            self.w.goOut()
            self.w.write(")")
        self.objectCount += 1

    def op_Integrator(self, op):
        if self.options.skipReg:
            return

        self.w.write("(registry '/renderer/common/type' 'direct')")# Ignore type for now
        self.w.write("(registry '/renderer/common/max_ray_depth %i)" % op.parameters['integer maxdepth'])

    def writeSampler(self, sampler, pixel_count):
        if self.options.skipReg:
            return

        self.w.write("(registry '/renderer/common/sampler/aa/type' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/sampler/aa/count' %i)" % pixel_count)
        self.w.write("(registry '/renderer/common/sampler/lens/type' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/sampler/lens/count' 1)")
        self.w.write("(registry '/renderer/common/sampler/time/type' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/sampler/time/count' 1)")

    def op_Sampler(self, op):
        if self.options.skipReg:
            return

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

    def writeFilter(self, filter, pixel_count):
        if self.options.skipReg:
            return
        self.w.write("(registry '/renderer/common/pixel/filter' '%s')" % sampler)
        self.w.write("(registry '/renderer/common/pixel/radius' %i)" % pixel_count)

    def op_PixelFilter(self, op):
        if self.options.skipReg:
            return

        xwidth = op.parameters["float xwidth"]
        ywidth = op.parameters["float ywidth"]
        count = int(max(xwidth, ywidth))

        if op.operand == "box":
            self.writeFilter("box", count)
        elif op.operand == "triangle":
            self.writeFilter("triangle", count)
        elif op.operand == "gaussian":
            self.writeFilter("gaussian", count)
        elif op.operand == "mitchell":
            self.writeFilter("mitchell", count)
        elif op.operand == "sinc":
            self.writeFilter("lanczos", count)
        else:
            self.writeFilter("mitchell", count)

    def op_Film(self, op):
        if self.options.skipReg:
            return

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
    def unpackRefl(col):
        if isinstance(col, list):
            return "(refl %f %f %f)" % (col[0], col[1], col[2])
        else:
            return "(refl %f %f %f)" % (col, col, col)

    @staticmethod
    def unpackIllum(col):
        if isinstance(col, list):
            return "(illum %f %f %f)" % (col[0], col[1], col[2])
        else:
            return "(illum %f %f %f)" % (col, col, col)

    def setupTexture(self, base, filename):
        if self.options.skipTex:
            return Operator.unpackRefl(1.0)

        name = "tex_%i" % self.textureCount
        self.textureCount += 1

        inc_file = self.resolveInclude(base, filename)

        self.w.write("(texture")
        self.w.goIn()
        self.w.write(":name '%s'" % name)
        self.w.write(":type 'color'")
        self.w.write(":file '%s'" % inc_file)
        self.w.goOut()
        self.w.write(")")

        return "(texture '%s')" % name

    def op_Material(self, op):
        if self.options.skipMat:
            return

        mat_type = op.parameters['string type']
        if mat_type == "matte":
            color = Operator.unpackRefl(op.parameters['rgb Kd'])
            self.w.write("(material")
            self.w.goIn()
            self.w.write(":type 'diffuse'")
            self.w.write(":name '%s'" % op.operand)
            self.w.write(":albedo %s" % color)
            self.w.goOut()
            self.w.write(")")
        elif mat_type == "disney":
            color = Operator.unpackRefl(op.parameters['rgb color'])
            self.w.write("(material")
            self.w.goIn()
            self.w.write(":type 'principled'")
            self.w.write(":name '%s'" % op.operand)
            self.w.write(":base %s" % color)
            self.w.write(":roughness %f" % op.parameters['float roughness'])
            self.w.write(":specular %f" % op.parameters['float spectrans'])
            self.w.write(":specular_tint %f" % op.parameters['float speculartint'])
            self.w.write(":metallic %f" % op.parameters['float metallic'])
            self.w.write(":clearcoat %f" % op.parameters['float clearcoat'])
            self.w.write(":clearcoat_gloss %f" % op.parameters['float clearcoatgloss'])
            self.w.write(":anisotropic %f" % op.parameters['float anisotropic'])
            self.w.write(":sheen %f" % op.parameters['float sheen'])
            self.w.write(":sheen_tint %f" % op.parameters['float sheentint'])
            #TODO: 'float eta', 'rgb scatterdistance' ?
            self.w.goOut()
            self.w.write(")")
        else:
            print("No support of materials of type %s available" % mat_type)

        self.currentMaterial = op.operand

    def op_MakeNamedMaterial(self, op):
        self.op_Material(op)# Same in our case

    def op_NamedMaterial(self, op):
        self.currentMaterial = op.operand

    def op_LightSource(self, op):
        if self.options.skipLight:
            return

        if op.operand == "point":
            pass # TODO
        elif op.operand == "infinite":
            tex_name = self.setupTexture(op.filename, op.parameters['string mapname'])
            self.w.write("(light")
            self.w.goIn()
            self.w.write(":type 'environment'")
            self.w.write(":radiance %s" % tex_name)
            self.w.goOut()
            self.w.write(")")
        elif op.operand == "distant":
            D = np.array(op.parameters['point to']) - np.array(op.parameters['point from'])
            D = D/np.linalg.norm(D, ord = 2)
            color = Operator.unpackIllum(op.parameters['rgb L'])

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
        if self.options.skipMesh:
            return

        name = "shape_%i" % self.shapeCount
        self.shapeCount += 1

        if op.operand == "plymesh":
            inc_file = Operator.resolve_filename(op.filename, op.parameters['string filename'])
            self.w.write("(embed")
            self.w.goIn()
            self.w.write(":loader 'ply'")
            self.w.write(":file '%s'" % inc_file)
            self.w.write(":name '%s'" % name)
            self.w.goOut()
            self.w.write(")")
            self.objectShapeAssoc[self.currentObject] = ('mesh', name, self.currentMaterial)
        elif op.operand == "trianglemesh":
            points = op.parameters['point P']
            normals = op.parameters.get('normal N')
            uv = op.parameters.get('point2 st')
            inds = op.parameters['integer indices']
            self.w.write("(mesh")
            self.w.goIn()
            self.w.write(":name '%s'" % name)
            # Vertices
            self.w.write("(attribute :type 'p'")
            self.w.goIn()
            line = ""
            for i in range(int(len(points)/3)):
                line += "[%f, %f, %f]," % (points[3*i+0],points[3*i+1],points[3*i+2])
            self.w.write(line)
            self.w.goOut()
            self.w.write(")")
            # Normals
            if normals is not None:
                self.w.write("(attribute :type 'n'")
                self.w.goIn()
                line = ""
                for i in range(int(len(normals)/3)):
                    line += "[%f, %f, %f]," % (normals[3*i+0],normals[3*i+1],normals[3*i+2])
                self.w.write(line)
                self.w.goOut()
                self.w.write(")")
            # UV
            if uv is not None:
                self.w.write("(attribute :type 'uv'")
                self.w.goIn()
                line = ""
                for i in range(int(len(uv)/2)):
                    line += "[%f, %f]," % (uv[2*i+0],uv[2*i+1])
                self.w.write(line)
                self.w.goOut()
                self.w.write(")")
            # Faces
            self.w.write("(faces")
            self.w.goIn()
            line = ""
            for i in range(int(len(inds)/3)):
                line += "[%i, %i, %i]," % (inds[3*i+0],inds[3*i+1],inds[3*i+2])
            self.w.write(line)
            self.w.goOut()
            self.w.write(")")
            self.w.goOut()
            self.w.write(")")
            self.objectShapeAssoc[self.currentObject] = ('mesh', name, self.currentMaterial)
        elif op.operand == "loopsubdiv":
            pass# TODO
        elif op.operand == "curve":
            pass# TODO
        elif op.operand == "sphere":
            self.objectShapeAssoc[self.currentObject] = ('sphere', op.parameters['radius'], self.currentMaterial)
        else:
            print("No support of shapes of type %s available" % op.operand)
