class Tokenizer:
    def __init__(self, source):
        self._tokens = []
        end = len(source)
        counter = 0
        last_item = ""
        while counter < end:
            if source[counter].isspace():
                if last_item != "":
                    self._tokens.append(last_item)
                    last_item = ""
                counter += 1
            elif source[counter] == '#':
                if last_item != "":
                    self._tokens.append(last_item)
                    last_item = ""
                while counter < end and source[counter] != '\n':
                    counter += 1
            elif source[counter] == '"':
                if last_item != "":
                    self._tokens.append(last_item)
                    last_item = ""
                item = '"'
                counter += 1
                while counter < end and source[counter] != '\n' and source[counter] != '"':
                    if source[counter] == '\\':  # Escape sequence
                        counter += 1
                        if counter >= end:
                            print("Invalid end of escape sequence")
                            return
                        if source[counter] == '\\':
                            item += '\\'
                        elif source[counter] == 'n':
                            item += '\n'
                        elif source[counter] == 't':
                            item += '\t'
                        elif source[counter] == 'r':
                            item += '\r'
                        elif source[counter] == 'f':
                            item += '\f'
                        elif source[counter] == '"':
                            item += '"'
                        else:
                            print("Invalid escape sequence \\%s" %
                                  source[counter])
                    else:
                        item += source[counter]
                    counter += 1
                item += '"'
                counter += 1
                self._tokens.append(item)
            elif source[counter] == '[':
                if last_item != "":
                    self._tokens.append(last_item)
                    last_item = ""
                self._tokens.append('[')
                counter += 1
            elif source[counter] == ']':
                if last_item != "":
                    self._tokens.append(last_item)
                    last_item = ""
                self._tokens.append(']')
                counter += 1
            else:
                last_item += source[counter]
                counter += 1

        if last_item != "":
            self._tokens.append(last_item)

        self._pos = 0

    def current(self):
        if self._pos < len(self._tokens):
            return self._tokens[self._pos]
        else:
            return None

    def next(self):
        self._pos += 1
        return self.current()

    def goBack(self):
        if self._pos > 0:
            self._pos -= 1
