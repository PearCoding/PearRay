class Tokenizer:
    def __init__(self, file):
        self._file = file
        self._current = None
        self._generator = self._tokens()
        self._retrieve_data()

    def _tokens(self):
        last_item = ""
        for line in self._file:
            #print(line)
            itr = iter(line)
            char = next(itr, None)
            while char is not None:
                if char.isspace():
                    if last_item != "":
                        yield last_item
                        last_item = ""
                    char = next(itr, None)
                elif char == '#':
                    if last_item != "":
                        yield last_item
                        last_item = ""
                    break # Give up on this line
                elif char == '"':
                    if last_item != "":
                        yield last_item
                        last_item = ""
                    item = '"'
                    char = next(itr, None)
                    while char is not None and char != '\n' and char != '"':
                        if char == '\\':  # Escape sequence
                            char = next(itr, None)
                            if counter >= end:
                                print("ERROR: Invalid end of escape sequence")
                                return
                            if char == '\\':
                                item += '\\'
                            elif char == 'n':
                                item += '\n'
                            elif char == 't':
                                item += '\t'
                            elif char == 'r':
                                item += '\r'
                            elif char == 'f':
                                item += '\f'
                            elif char == '"':
                                item += '"'
                            else:
                                print("ERROR: Invalid escape sequence \\%s" %
                                    char)
                        else:
                            item += char
                        char = next(itr, None)
                    item += '"'
                    yield item
                    char = next(itr, None)
                elif char == '[':
                    if last_item != "":
                        yield last_item
                        last_item = ""
                    yield '['
                    char = next(itr, None)
                elif char == ']':
                    if last_item != "":
                        yield last_item
                        last_item = ""
                    yield ']'
                    char = next(itr, None)
                else:
                    last_item += char
                    char = next(itr, None)
        if last_item != "":
            yield last_item

    def _retrieve_data(self):
        try:
            self._current = next(self._generator)
            #print(self._current)
        except StopIteration:
            self._current = None

    def current(self):
        return self._current

    def next(self):
        self._retrieve_data()
        return self.current()

