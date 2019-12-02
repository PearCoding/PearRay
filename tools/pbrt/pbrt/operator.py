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

    def apply(self, operations):
        # print(str(operations))
        for op in operations:
            try:
                method = getattr(self, "op_" + op.action)
                method(op)
            except AttributeError:
                print("Invalid Action %s" % op.action)

    @staticmethod
    def mat2str(mat):
        return "[" + ",".join(",".join(f for f in row) for row in mat) + "]"

    def op_Include(self, op):
        file_dir = os.path.dirname(op.filename)
        inc_file = op.operand if os.path.isabs(
            op.operand) else os.path.join(file_dir, op.operand)

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

    def op_Translate(self, op):
        self.transformStack[-1] *= np.array([[1, 0, 0, op.operand[0]],
                                             [0, 1, 0, op.operand[1]],
                                             [0, 0, 1, op.operand[2]],
                                             [0, 0, 0, 1]])

    def op_Rotate(self, op):
        co = math.cos(op.operand[0])
        si = math.sin(op.operand[0])

        x = op.operand[1]
        y = op.operand[2]
        z = op.operand[3]

        self.transformStack[-1] *= np.array([[co+x*x*(1-co), x*y*(1-co)-z*si, x*z*(1-co)+y*si, 0],
                                             [y*x*(1-co)+z*si, co+y*y*(1-co), y*z*(1-co)-x*si, 0],
                                             [z*x*(1-co)-y*si, z*y*(1-co)+x*si, co+z*z*(1-co), 0],
                                             [0, 0, 0, 1]])

    def op_Scale(self, op):
        self.transformStack[-1] *= np.array([[op.operand[0], 0, 0, 0],
                                             [0, op.operand[1], 0, 0],
                                             [0, 0, op.operand[2], 0],
                                             [0, 0, 0, 1]])

    def op_Transform(self, op):
        self.transformStack[-1] = np.array([op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]])
        print(self.transformStack[-1])

    def op_ConcatTransform(self, op):
        self.transformStack[-1] *= np.array([op.operand[0:4], op.operand[4:8], op.operand[8:12], op.operand[12:16]])

    def op_CoordinateSystem(self, op):
        self.coords[op.operand[0]] = self.transformStack[-1]

    def op_CoordinateSysTransform(self, op):
        self.transformStack[-1] = self.coords[op.operand[0]]

    def op_Camera(self, op):
        self.coords['camera'] = self.transformStack[-1]

        # Only support one camera
        self.w.write(":camera 'Camera'")
        self.w.write("(camera")
        self.w.goIn()
        self.w.write(":name 'Camera'")
        self.w.write(":type 'standard'")
        self.w.write(":transform %s" % Operator.mat2str(self.transformStack[-1]))
        self.w.goOut()
        self.w.write(")")

    def op_WorldBegin(self, op):
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

    def op_ObjectInstance(self, op):
        if self.currentObject != "":
            print("Already inside object definition")

        # TODO: Add simple copy instance support to PearRay!