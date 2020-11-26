import os
import numpy as np
import math

from .parser import Parser
from .writer import Writer
from . import colexport, matexport, objexport, texexport


TRANSFORM = np.array([[1, 0, 0],
                      [0, 0, 1],
                      [0, 1, 0]])

TRANSFORM_CAM = np.array([[1, 0, 0, 0],
                          [0, 0, 1, 0],
                          [0, 1, 0, 0],
                          [0, 0, 0, 1]])


class Operator:
    class Context:
        def __init__(self):
            self.transform = np.identity(4)
            self.areaLight = ""

        def copy(self):
            ctx = Operator.Context()
            ctx.transform = np.copy(self.transform)
            ctx.areaLight = self.areaLight
            return ctx

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
        self.matCount = 0
        self.objectShapeAssoc = {}
        self.includes = {}
        self.aspectRatio = None

        # The following warnings can be annoying, so show them only once
        self.warnedAboutCurves = False
        self.warnedAboutSubdiv = False

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

    def resolveFilename(self, base, filename):
        file_dir = os.path.dirname(base) if os.path.isfile(base) else base
        return filename if os.path.isabs(
            filename) else os.path.normpath(os.path.join(os.path.join(file_dir, self.options.include_offset), filename))

    def resolveInclude(self, base, filename):
        inc_file = self.resolveFilename(base, filename)
        if not os.path.exists(inc_file):
            inc_file = self.resolveFilename(self.globalDir, filename)
        return inc_file

    def op_Include(self, op):
        inc_file = self.resolveInclude(op.filename, op.operand)
        if not self.options.quite:
            print("Including: %s" % inc_file)

        if inc_file not in self.includes:
            self.includes[inc_file] = 1
        else:
            self.includes[inc_file] += 1
            if self.options.skipRepetitiveIncludes:
                if not self.options.quite:
                    print("Skipping include: %s" % inc_file)
                return

        old_writer = self.w
        if not self.options.singleFile:
            rel_file = os.path.relpath(
                os.path.abspath(inc_file), self.globalDir)

            post_fix = '' if self.includes[inc_file] == 1 else '_%i' % (
                self.includes[inc_file])

            output_file = os.path.join(
                self.options.output, os.path.splitext(rel_file)[0]+post_fix+'.prc')
            os.makedirs(os.path.dirname(output_file), exist_ok=True)

            if not self.options.quite:
                print("-> %s" % output_file)

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
        self.contextStack[-1].transform = np.matmul(
            self.contextStack[-1].transform, mat)
        # print(self.contextStack[-1].transform)

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
                                      [y*x*(1-co)+z*si, co+y*y*(1-co),
                                       y*z*(1-co)-x*si, 0],
                                      [z*x*(1-co)-y*si, z*y*(1-co) +
                                       x*si, co+z*z*(1-co), 0],
                                      [0, 0, 0, 1]]))

    def op_Scale(self, op):
        self.applyTransform(np.array([[op.operand[0], 0, 0, 0],
                                      [0, op.operand[1], 0, 0],
                                      [0, 0, op.operand[2], 0],
                                      [0, 0, 0, 1]]))

    @staticmethod
    def matFromOp(op):
        return np.transpose(np.array(
            [op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]]))

    def op_Transform(self, op):
        self.contextStack[-1].transform = Operator.matFromOp(op)
        # print(self.contextStack[-1].transform)

    def op_ConcatTransform(self, op):
        self.applyTransform(Operator.matFromOp(op))
        # print(self.contextStack[-1].transform)

    @staticmethod
    def normVec(v):
        norm = np.linalg.norm(v)
        if norm == 0:
            return v
        return v / norm

    def op_LookAt(self, op):
        pos = np.array(op.operand[0:3])
        lookPos = np.array(op.operand[3:6])
        up = np.array(op.operand[6:9])

        dir = Operator.normVec(lookPos-pos)
        right = Operator.normVec(np.cross(Operator.normVec(up), dir, axis=0))
        up = np.cross(dir, right, axis=0)

        CM = np.array([[right[0], up[0], dir[0], pos[0]],
                       [right[1], up[1], dir[1], pos[1]],
                       [right[2], up[2], dir[2], pos[2]],
                       [0, 0, 0, 1]])
        self.applyTransform(np.linalg.inv(CM))

    def op_CoordinateSystem(self, op):
        self.coords[op.operand] = np.copy(self.contextStack[-1].transform)

    def op_CoordinateSysTransform(self, op):
        self.contextStack[-1].transform = np.copy(self.coords[op.operand])

    def op_Camera(self, op):
        cam_mat = np.linalg.inv(self.contextStack[-1].transform)

        w = 1
        h = 1
        if 'fov' in op.parameters:
            h = 2*math.tan(math.radians(op.parameters['fov'])/2)
            if self.aspectRatio is None:
                print("Can not detect aspect ratio at the moment!")
                w = h
            else:
                if self.aspectRatio > 1:
                    w = h*self.aspectRatio
                else:
                    w = h
                    h = w/self.aspectRatio

        cam_mat = cam_mat @ TRANSFORM_CAM
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
        self.w.write(":height %f" % h)
        self.w.write(":transform %s" % Operator.mat2str(cam_mat))
        self.w.goOut()
        self.w.write(")")

    def op_WorldBegin(self, op):
        self.op_Identity(op)
        self.coords['world'] = np.copy(self.contextStack[-1].transform)

    def op_WorldEnd(self, op):
        while len(self.contextStack) != 1:
            self.contextStack.pop()

    def op_AttributeBegin(self, op):
        self.contextStack.append(self.contextStack[-1].copy())

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
                if shape[2] == '':
                    self.w.write(":camera_visible false")
                    self.w.write(":light_visible false")
                    self.w.write(":bounce_visible false")
            self.w.write(":mesh '%s'" % shape[1])
            self.w.write(":transform %s" % Operator.mat2str(
                self.contextStack[-1].transform))
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
                if shape[2] == '':
                    self.w.write(":camera_visible false")
                    self.w.write(":light_visible false")
                    self.w.write(":bounce_visible false")
            self.w.write(":radius %f" % shape[1])
            self.w.write(":transform %s" % Operator.mat2str(
                self.contextStack[-1].transform))
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
        if self.options.skipWorld:
            return

        # Ignore type for now
        self.w.write("(integrator")
        self.w.goIn()
        self.w.write(":type 'direct'")
        self.w.write(":max_ray_depth %i" % op.parameters.get('maxdepth', 8))
        self.w.goOut()
        self.w.write(")")

    def writeSampler(self, sampler, pixel_count):
        self.w.write("(sampler")
        self.w.goIn()
        self.w.write(":slot 'aa'")
        self.w.write(":type '%s'" % sampler)
        self.w.write(":sample_count %i" % pixel_count)
        self.w.goOut()
        self.w.write(")")

    def op_Sampler(self, op):
        if self.options.skipWorld:
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
        pass  # Ignoring for now

    def writeFilter(self, filter, pixel_count):
        self.w.write("(filter")
        self.w.goIn()
        self.w.write(":slot 'pixel'")
        self.w.write(":type '%s'" % filter)
        self.w.write(":radius %i" % pixel_count)
        self.w.goOut()
        self.w.write(")")

    def op_PixelFilter(self, op):
        if self.options.skipWorld:
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
        if self.options.skipWorld:
            return

        if op.operand != "image":
            print("ERROR: Only support image output!")
            return

        if self.aspectRatio is not None:
            print("WARNING: Only support one film! Ignoring")
            return

        w = op.parameters["xresolution"]
        h = op.parameters["yresolution"]
        self.aspectRatio = float(w) / float(h)

        filename = os.path.splitext(
            os.path.basename(op.parameters["filename"]))[0]
        self.w.write(":render_width %i" % w)
        self.w.write(":render_height %i" % h)
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
        pass  # Ignoring for now

    def op_MediumInterface(self, op):
        pass  # Ignoring for now

    def op_Texture(self, op):
        if self.options.skipTex:
            return

        texexport.export(self, op)

    def setupTexture(self, base, filename):
        if self.options.skipTex:
            return "'%s'" % colexport.unpackRefl(self, 1.0)

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
        self.currentMaterial = matexport.export(self, op, isNamed=False)

    def op_MakeNamedMaterial(self, op):
        if self.options.skipMat:
            return
        self.currentMaterial = matexport.export(self, op, isNamed=True)

    def op_NamedMaterial(self, op):
        self.currentMaterial = op.operand

    def op_LightSource(self, op):
        if self.options.skipLight:
            return

        if op.operand == "point":
            print("WARNING: Point light sources not yet implemented!")  # TODO
        elif op.operand == "infinite":
            tex_name = None
            if 'mapname' in op.parameters:
                tex_name = self.setupTexture(
                    op.filename, op.parameters['mapname'])

            self.w.write("(light")
            self.w.goIn()
            self.w.write(":type 'environment'")
            self.w.write(":transform  %s" % Operator.mat2str(TRANSFORM))
            if tex_name is not None:
                self.w.write(
                    ":radiance (smul (illuminant 'd65') %s)" % tex_name)
            else:
                self.w.write(":radiance (smul (illuminant 'd65') %s)" %
                             colexport.unpackIllum(self, op.parameters['L']))
            self.w.goOut()
            self.w.write(")")
        elif op.operand == "distant":
            D = np.array(op.parameters['from'])-np.array(op.parameters['to'])
            D = D/np.linalg.norm(D, ord=2)
            D = TRANSFORM @ D
            color = colexport.unpackIllum(self, op.parameters['L'])

            self.w.write("(light")
            self.w.goIn()
            self.w.write(":type 'distant'")
            self.w.write(":direction [%f, %f, %f]" % (D[0], D[1], D[2]))
            self.w.write(":radiance (smul (illuminant 'd65') %s)" % color)
            self.w.goOut()
            self.w.write(")")
        else:
            print("ERROR: No support of light sources of type %s available" % op.operand)

    def op_AreaLightSource(self, op):
        name = "emission_%i" % self.emsCount
        self.contextStack[-1].areaLight = name
        self.emsCount += 1

        if op.operand == 'diffuse':
            color = colexport.unpackIllum(self, op.parameters['L'])

            self.w.write("(emission")
            self.w.goIn()
            self.w.write(":name '%s'" % name)
            self.w.write(":type 'diffuse'")
            self.w.write(":radiance (smul (illuminant 'd65') %s)" % color)
            self.w.goOut()
            self.w.write(")")
        else:
            print(
                "ERROR: No support of area light sources of type %s available" % op.operand)

    def writeVectors(self, vecs, elemS):
        elmsW = 0
        le = int(len(vecs)/elemS)

        line = ""
        for i in range(le):
            line += "[%s]," % (",".join(str(v)
                                        for v in vecs[elemS*i:elemS*(i+1)]))
            elmsW += 1
            if elmsW > 200:
                self.w.write(line)
                elmsW = 0
                line = ""

        if elmsW > 0:
            self.w.write(line)

    def op_Shape(self, op):
        isDef = self.currentObject != ''
        name = "shape_%i" % self.shapeCount

        if op.operand == "plymesh":
            if self.options.skipMesh:
                return

            inc_file = self.resolveFilename(
                op.filename, op.parameters['filename'])
            self.w.write("(embed")
            self.w.goIn()
            self.w.write(":loader 'ply'")
            self.w.write(":file '%s'" % inc_file)
            self.w.write(":name '%s'" % name)
            self.w.goOut()
            self.w.write(")")

            shape = ('mesh', name, self.currentMaterial,
                     self.contextStack[-1].areaLight)
            if isDef:
                self.objectShapeAssoc[self.currentObject].append(shape)
            else:
                self.writeShape(shape)
            self.shapeCount += 1
        elif op.operand == "trianglemesh":
            if self.options.skipMesh:
                return

            points = op.parameters['P']
            normals = op.parameters.get('N')
            uv = op.parameters.get('uv')
            inds = op.parameters['indices']
            if self.options.embedMesh or len(points) < 1000:
                name = name + "_e"
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
            else:  # Obj file
                name = name + "_o"
                rel_file = os.path.relpath(
                    os.path.abspath(op.filename), self.globalDir)
                obj_file = os.path.join(
                    self.options.output,
                    os.path.join(os.path.dirname(rel_file), name + '.obj'))
                if not self.options.quite:
                    print("Exporting mesh to %s" % obj_file)
                objexport.export(obj_file, points, normals, uv, inds)
                self.w.write("(embed")
                self.w.goIn()
                self.w.write(":loader 'obj'")
                self.w.write(":file '%s'" % obj_file)
                self.w.write(":name '%s'" % name)
                self.w.goOut()
                self.w.write(")")

            shape = ('mesh', name, self.currentMaterial,
                     self.contextStack[-1].areaLight)
            if isDef:
                self.objectShapeAssoc[self.currentObject].append(shape)
            else:
                self.writeShape(shape)
            self.shapeCount += 1
        elif op.operand == "loopsubdiv":
            if not self.warnedAboutSubdiv:
                print("ERROR: loopsubdiv - Not yet supported")
                self.warnedAboutSubdiv = True
        elif op.operand == "curve":
            if self.options.skipCurve:
                return
            if not self.warnedAboutCurves:
                print("ERROR: curve - Not yet supported")
                self.warnedAboutCurves = True
        elif op.operand == "sphere":
            if self.options.skipPrim:
                return
            shape = ('sphere', op.parameters['radius'],
                     self.currentMaterial, self.contextStack[-1].areaLight)
            if isDef:
                self.objectShapeAssoc[self.currentObject].append(shape)
            else:
                self.writeShape(shape)
            self.shapeCount += 1
        else:
            print("ERROR: No support of shapes of type %s available" % op.operand)
