class Writer:
    def __init__(self, file, indent=True):
        self.file = file
        self.indent = indent
        self.currentLevel = 0
        self.indentObj = "  "


    def write(self, content):
        prefix = ""
        if self.indent:
            for i in range(self.currentLevel):
                prefix = prefix + self.indentObj

        self.file.write(prefix + content + "\n")


    def goIn(self):
        self.currentLevel = self.currentLevel + 1


    def goOut(self):
        self.currentLevel = self.currentLevel - 1
        if self.currentLevel < 0:
            print("DEV ERROR: PEARRAY Exporter currentLevel < 0!")
