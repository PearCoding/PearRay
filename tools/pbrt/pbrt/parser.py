from .tokenizer import Tokenizer


class Operation:
    def __init__(self, filename, action, operand, params):
        self.filename = filename
        self.action = action
        self.operand = operand
        self.parameters = params

    def hasOperand(self):
        return self.operand is not None

    def param(self, name):
        return self.parameters[name]

    def __repr__(self):
        return "<%s[%s]: %s>" % (self.action, self.operand, self.parameters)


class Parser:
    @staticmethod
    def isFloat(token):
        try:
            float(token)
            return True
        except ValueError:
            return False

    @staticmethod
    def isInteger(token):
        try:
            int(token)
            return True
        except ValueError:
            return False

    @staticmethod
    def isParameter(token):
        return token is not None and (token.startswith('"') or Parser.isFloat(token) or Parser.isInteger(token))

    @staticmethod
    def isParameterName(token):
        return token is not None and token.startswith('"')

    @staticmethod
    def getParameter(token):
        if token.startswith('"'):
            return token[1:-1]
        elif Parser.isInteger(token):
            return int(token)
        elif Parser.isFloat(token):
            return float(token)
        else:
            print("Invalid parameter given")
            return None

    @staticmethod
    def isListStart(token):
        return token is not None and token.startswith('[')

    @staticmethod
    def isListEnd(token):
        return token is not None and token.startswith(']')

    def parse_parameter(self, tokenizer):
        token = tokenizer.current()
        if Parser.isListStart(token):
            token = tokenizer.next()
            paramList = []
            while Parser.isParameter(token):
                paramList.append(Parser.getParameter(token))
                token = tokenizer.next()
            if not Parser.isListEnd(token):
                print("Bad list end")
            tokenizer.next() # Skip ]
            return paramList
        elif Parser.isParameter(token):
            tokenizer.next()
            return Parser.getParameter(token)
        else:
            return None

    def parse_parameter_list(self, tokenizer):
        token = tokenizer.current()
        params = {}
        while Parser.isParameterName(token):
            tokenizer.next()
            params[token] = self.parse_parameter(tokenizer)
            token = tokenizer.current()

        return params

    def parse_operand(self, tokenizer):
        return self.parse_parameter(tokenizer)

    def parse_action(self, tokenizer):
        action = tokenizer.current()
        tokenizer.next()
        return action

    def parse(self, filename, source):
        tokenizer = Tokenizer(source)

        operations = []
        while True:
            action = self.parse_action(tokenizer)
            if action is None:
                break

            operand = self.parse_operand(tokenizer)
            params = {}
            if operand is not None:
                params = self.parse_parameter_list(tokenizer)

            operations.append(Operation(filename, action, operand, params))

        return operations

