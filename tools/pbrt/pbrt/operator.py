import os
import numpy as np
import math

from .parser import Parser
from .writer import Writer


class Operator:
    class Context:
        def __init__(self):
            self.transform = np.identity(4)
            self.areaLight = ""

    def __init__(self, writer, opts, global_dir):
        self.options = opts
        self.globalDir = global_dir

        self.w = writer
        self.coords = {}
        self.contextStack = [Operator.Context()]
        self.currentObject = ""
        self.currentMaterial = ""
        self.objectCount = 0
        self.shapeCount = 0
        self.emsCount = 0
        self.specCount = 0
        self.textureCount = 0
        self.objectShapeAssoc = {}

    def apply(self, op):
        try:
            method = getattr(self, "op_" + op.action)
        except AttributeError:
            print("ERROR: Unknown Action %s" % op.action)
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
        if not self.options.quite:
            print("Including: %s" % inc_file)

        old_writer = self.w
        if not self.options.singleFile:
            rel_file = os.path.relpath(os.path.abspath(inc_file), self.globalDir)
            output_file = os.path.join(self.options.output, os.path.splitext(rel_file)[0]+'.prc')
            os.makedirs(os.path.dirname(output_file), exist_ok=True)

            self.w.write("(include '%s')" % output_file)
            self.w.flush()
            self.w = Writer(open(output_file, "w"))

        try:
            with open(inc_file, "r") as file:
                for op in Parser.parse(inc_file, file):
                    self.apply(op)
        except IOError as e:
            print(e)
            return

        self.w = old_writer

    def op_Identity(self, op):
        self.contextStack[-1].transform = np.identity(4)

    def applyTransform(self, mat):
        self.contextStack[-1].transform = np.dot(self.contextStack[-1].transform, mat)
        #print(self.contextStack[-1].transform)

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
        self.contextStack[-1].transform = np.transpose(np.array([op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]]))
        #print(self.contextStack[-1].transform)

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
        self.coords[op.operand] = self.contextStack[-1].transform

    def op_CoordinateSysTransform(self, op):
        self.contextStack[-1].transform = self.coords[op.operand]

    def op_Camera(self, op):
        cam_mat =  np.linalg.inv(self.contextStack[-1].transform)

        w = 1
        if 'fov' in op.parameters:
            w = math.tan(math.radians(op.parameters['fov']))

        cam_mat = np.dot(cam_mat, np.array([[1, 0, 0, 0],
                                            [0,-1, 0, 0],
                                            [0, 0, 1, 0],
                                            [0, 0, 0, 1]]))
        self.coords['camera'] = cam_mat

        if self.options.skipCamera:
            return

        # Only support one camera
        self.w.write(":camera 'Camera'")
        self.w.write("(camera")
        self.w.goIn()
        self.w.write(":name 'Camera'")
        self.w.write(":type 'standard'")
        self.w.write(":width %f" % w)
        self.w.write(":height %f" % w)
        self.w.write(":transform %s" % Operator.mat2str(cam_mat))
        self.w.goOut()
        self.w.write(")")

    def op_WorldBegin(self, op):
        self.op_Identity(op)
        self.coords['world'] = self.contextStack[-1].transform

    def op_WorldEnd(self, op):
        while len(self.contextStack) != 1:
            self.contextStack.pop()

    def op_AttributeBegin(self, op):
        self.contextStack.append(self.contextStack[-1])

    def op_AttributeEnd(self, op):
        if len(self.contextStack) == 1:
            print("ERROR: Can not pop transform stack anymore!")
        else:
            self.contextStack.pop()

    def op_TransformBegin(self, op):
        self.op_AttributeBegin(op)

    def op_TransformEnd(self, op):
        self.op_AttributeEnd(op)

    def op_ObjectBegin(self, op):
        if self.currentObject != "":
            print("ERROR: Already inside object definition")

        self.currentObject = op.operand
        self.objectShapeAssoc[self.currentObject] = []
        self.op_AttributeBegin(op)

    def op_ObjectEnd(self, op):
        if self.currentObject == "":
            print("ERROR: Already outside object definition")

        self.currentObject = ""
        self.op_AttributeEnd(op)

    def writeShape(self, shape):
        if shape[0] == 'mesh':
            self.w.write("(entity")
            self.w.goIn()
            self.w.write(":name 'object_%i'" % self.objectCount)
            self.w.write(":type 'mesh'")
            if shape[2] != '':
                self.w.write(":materials ['%s']" % shape[2])
            if shape[3] != '':
                self.w.write(":emission '%s'" % shape[3])
            self.w.write(":mesh '%s'" % shape[1])
            self.w.write(":transform %s" % Operator.mat2str(self.contextStack[-1].transform))
            self.w.goOut()
            self.w.write(")")
        elif shape[0] == 'sphere':
            self.w.write("(entity")
            self.w.goIn()
            self.w.write(":name 'object_%i'" % self.objectCount)
            self.w.write(":type 'sphere'")
            if shape[2] != '':
                self.w.write(":material '%s'" % shape[2])
            if shape[3] != '':
                self.w.write(":emission '%s'" % shape[3])
            self.w.write(":radius %f" % shape[1])
            self.w.write(":transform %s" % Operator.mat2str(self.contextStack[-1].transform))
            self.w.goOut()
            self.w.write(")")
        self.objectCount += 1

    def op_ObjectInstance(self, op):
        if self.currentObject != "":
            print("ERROR: Already inside object definition")
            return

        if self.options.skipInstance:
            return

        if not op.operand in self.objectShapeAssoc:
            print("ERROR: Unknown object %s" % op.operand)
            return

        assoc = self.objectShapeAssoc[op.operand]

        for shape in assoc:
            self.writeShape(shape)

    def op_Integrator(self, op):
        if self.options.skipReg:
            return

        self.w.write("(registry '/renderer/common/type' 'direct')")# Ignore type for now
        self.w.write("(registry '/renderer/common/max_ray_depth' %i)" % op.parameters['maxdepth'])

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

        count = op.parameters["pixelsamples"]
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
        self.w.write("(registry '/renderer/common/pixel/filter' '%s')" % filter)
        self.w.write("(registry '/renderer/common/pixel/radius' %i)" % pixel_count)

    def op_PixelFilter(self, op):
        if self.options.skipReg:
            return

        xwidth = op.parameters["xwidth"]
        ywidth = op.parameters["ywidth"]
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
            print("ERROR: Only support image output!")
            return

        filename = os.path.splitext(os.path.basename(op.parameters["filename"]))[0]
        self.w.write(":renderWidth %i" % op.parameters["xresolution"])
        self.w.write(":renderHeight %i" % op.parameters["yresolution"])
        self.w.write("(output")
        self.w.goIn()
        self.w.write(":name '%s'" % filename)
        self.w.write("(channel")
        self.w.goIn()
        self.w.write(":type 'color'")
        self.w.write(":color 'rgb'")
        self.w.write(":gamma 'none'")
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

    def unpackRefl(self, col):
        name = "spec_%i" % self.specCount
        self.specCount += 1

        self.w.write("(spectrum")
        self.w.goIn()
        self.w.write(":name '%s'" % name)
        if isinstance(col, list):
            self.w.write(":data (refl %f %f %f)" % (col[0], col[1], col[2]))
        else:
            self.w.write(":data (refl %f %f %f)" % (col, col, col))
        self.w.goOut()
        self.w.write(")")
        return name

    def unpackIllum(self, col):
        name = "spec_%i" % self.specCount
        self.specCount += 1

        self.w.write("(spectrum")
        self.w.goIn()
        self.w.write(":name '%s'" % name)
        if isinstance(col, list):
            self.w.write(":data (illum %f %f %f)" % (col[0], col[1], col[2]))
        else:
            self.w.write(":data (illum %f %f %f)" % (col, col, col))
        self.w.goOut()
        self.w.write(")")
        return name

    def setupTexture(self, base, filename):
        if self.options.skipTex:
            return "'%s'" % self.unpackRefl(1.0)

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

        mat_type = op.parameters['type']
        if mat_type == "matte":
            color = self.unpackRefl(op.parameters['Kd'])
            self.w.write("(material")
            self.w.goIn()
            self.w.write(":type 'diffuse'")
            self.w.write(":name '%s'" % op.operand)
            self.w.write(":albedo '%s'" % color)
            self.w.goOut()
            self.w.write(")")
        elif mat_type == "disney":
            color = self.unpackRefl(op.parameters['color'])
            self.w.write("(material")
            self.w.goIn()
            self.w.write(":type 'principled'")
            self.w.write(":name '%s'" % op.operand)
            self.w.write(":base '%s'" % color)
            self.w.write(":roughness %f" % op.parameters['roughness'])
            self.w.write(":specular %f" % op.parameters['spectrans'])
            self.w.write(":specular_tint %f" % op.parameters['speculartint'])
            self.w.write(":metallic %f" % op.parameters['metallic'])
            self.w.write(":clearcoat %f" % op.parameters['clearcoat'])
            self.w.write(":clearcoat_gloss %f" % op.parameters['clearcoatgloss'])
            self.w.write(":anisotropic %f" % op.parameters['anisotropic'])
            self.w.write(":sheen %f" % op.parameters['sheen'])
            self.w.write(":sheen_tint %f" % op.parameters['sheentint'])
            #TODO: 'eta', 'scatterdistance' ?
            self.w.goOut()
            self.w.write(")")
        else:
            print("ERROR: No support of materials of type %s available" % mat_type)

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
            tex_name = self.setupTexture(op.filename, op.parameters['mapname'])
            self.w.write("(light")
            self.w.goIn()
            self.w.write(":type 'environment'")
            self.w.write(":radiance %s" % tex_name)
            self.w.goOut()
            self.w.write(")")
        elif op.operand == "distant":
            D = np.array(op.parameters['to']) - np.array(op.parameters['from'])
            D = D/np.linalg.norm(D, ord = 2)
            color = self.unpackIllum(op.parameters['L'])

            self.w.write("(light")
            self.w.goIn()
            self.w.write(":type 'distant'")
            self.w.write(":direction [%f, %f, %f]" % (D[0], D[1], D[2]))
            self.w.write(":radiance '%s'" % color)
            self.w.goOut()
            self.w.write(")")
        else:
            print("ERROR: No support of light sources of type %s available" % op.operand)

    def op_AreaLightSource(self, op):
        name = "emission_%i" % self.emsCount
        self.contextStack[-1].areaLight = name
        self.emsCount += 1

        if op.operand == 'diffuse':
            color = self.unpackIllum(op.parameters['L'])

            self.w.write("(emission")
            self.w.goIn()
            self.w.write(":name '%s'" % name)
            self.w.write(":type 'diffuse'")
            self.w.write(":radiance '%s'" % color)
            self.w.goOut()
            self.w.write(")")
        else:
            print("ERROR: No support of area light sources of type %s available" % op.operand)

    def writeVectors(self, vecs, elemS):
        elmsW = 0
        le = int(len(vecs)/elemS)

        line = ""
        for i in range(le):
            line += "[%s]," % (",".join(str(v) for v in vecs[elemS*i:elemS*(i+1)]))
            elmsW += 1
            if elmsW > 200:
                self.w.write(line)
                elmsW = 0
                line = ""

        if elmsW > 0:
            self.w.write(line)

    def op_Shape(self, op):
        if self.options.skipMesh:
            return

        isDef = self.currentObject != ''

        name = "shape_%i" % self.shapeCount
        self.shapeCount += 1

        if op.operand == "plymesh":
            inc_file = Operator.resolveFilename(op.filename, op.parameters['filename'])
            self.w.write("(embed")
            self.w.goIn()
            self.w.write(":loader 'ply'")
            self.w.write(":file '%s'" % inc_file)
            self.w.write(":name '%s'" % name)
            self.w.goOut()
            self.w.write(")")

            shape = ('mesh', name, self.currentMaterial, self.contextStack[-1].areaLight)
            if isDef:
                self.objectShapeAssoc[self.currentObject].append(shape)
            else:
                self.writeShape(shape)
        elif op.operand == "trianglemesh":
            points = op.parameters['P']
            normals = op.parameters.get('N')
            uv = op.parameters.get('st')
            inds = op.parameters['indices']
            self.w.write("(mesh")
            self.w.goIn()
            self.w.write(":name '%s'" % name)
            # Vertices
            self.w.write("(attribute :type 'p'")
            self.w.goIn()
            self.writeVectors(points, 3)
            self.w.goOut()
            self.w.write(")")
            # Normals
            if normals is not None:
                self.w.write("(attribute :type 'n'")
                self.w.goIn()
                self.writeVectors(normals, 3)
                self.w.goOut()
                self.w.write(")")
            # UV
            if uv is not None:
                self.w.write("(attribute :type 'uv'")
                self.w.goIn()
                self.writeVectors(uv, 2)
                self.w.goOut()
                self.w.write(")")
            # Faces
            self.w.write("(faces")
            self.w.goIn()
            self.writeVectors(inds, 3)
            self.w.goOut()
            self.w.write(")")
            self.w.goOut()
            self.w.write(")")

            shape = ('mesh', name, self.currentMaterial, self.contextStack[-1].areaLight)
            if isDef:
                self.objectShapeAssoc[self.currentObject].append(shape)
            else:
                self.writeShape(shape)
        elif op.operand == "loopsubdiv":
            print("ERROR: loopsubdiv - Not yet supported")
        elif op.operand == "curve":
            print("ERROR: curve - Not yet supported")
        elif op.operand == "sphere":
            shape = ('sphere', op.parameters['radius'], self.currentMaterial, self.contextStack[-1].areaLight)
            if isDef:
                self.objectShapeAssoc[self.currentObject].append(shape)
            else:
                self.writeShape(shape)
        else:
            print("ERROR: No support of shapes of type %s available" % op.operand)
