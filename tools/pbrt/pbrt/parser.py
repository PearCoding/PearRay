from .tokenizer import Tokenizer
from .operation import Operation

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
    def isNumber(token):
        return Parser.isFloat(token) or Parser.isInteger(token)

    @staticmethod
    def isParameter(token):
        return token is not None and (token.startswith('"') or Parser.isNumber(token))

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
            print("ERROR: Invalid parameter given")
            return None

    @staticmethod
    def isListStart(token):
        return token is not None and token.startswith('[')

    @staticmethod
    def isListEnd(token):
        return token is not None and token.startswith(']')

    @staticmethod
    def parse_parameter(tokenizer):
        token = tokenizer.current()
        if Parser.isListStart(token):
            token = tokenizer.next()
            paramList = []
            while Parser.isParameter(token):
                paramList.append(Parser.getParameter(token))
                token = tokenizer.next()
            if not Parser.isListEnd(token):
                print("ERROR: Bad list end")
            tokenizer.next() # Skip ]
            if len(paramList) == 1:
                return paramList[0]
            else:
                return paramList
        elif token is not None and Parser.isNumber(token):
            # For lookAt and transform actions which uses no [ ]
            paramList = []
            while token is not None and Parser.isNumber(token):
                paramList.append(Parser.getParameter(token))
                token = tokenizer.next()
            if len(paramList) == 1:
                return paramList[0]
            else:
                return paramList
        elif Parser.isParameter(token):
            tokenizer.next()
            return Parser.getParameter(token)
        else:
            return None

    @staticmethod
    def parse_parameter_list(tokenizer):
        token = tokenizer.current()
        params = {}
        while Parser.isParameterName(token):
            tokenizer.next()
            params[Parser.getParameter(token)] = Parser.parse_parameter(tokenizer)
            token = tokenizer.current()

        return params

    @staticmethod
    def parse_operand(tokenizer):
        return Parser.parse_parameter(tokenizer)

    @staticmethod
    def parse_action(tokenizer):
        action = tokenizer.current()
        tokenizer.next()
        return action

    @staticmethod
    def parse(filename, file):
        tokenizer = Tokenizer(file)

        while True:
            action = Parser.parse_action(tokenizer)
            if action is None:
                break

            operand = Parser.parse_operand(tokenizer)
            params = {}
            if operand is not None:
                params = Parser.parse_parameter_list(tokenizer)

            yield Operation(filename, action, operand, params)
