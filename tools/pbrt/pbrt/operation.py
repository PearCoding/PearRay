class Operation:
    def __init__(self, filename, action, operand, params):
        self.filename = filename
        self.action = action
        self.operand = operand
        self.parameters = {' '.join(n.split()[1:]): v for n, v in params.items()}

    def hasOperand(self):
        return self.operand is not None

    def param(self, name):
        return self.parameters[name]

    def __repr__(self):
        return "<%s[%s]: %s>" % (self.action, self.operand, self.parameters)
