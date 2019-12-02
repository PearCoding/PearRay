class Writer:
    def __init__(self, file, useTabs=True):
        self.file = file
        self.useTabs = useTabs
        self.currentLevel = 0


    def write(self, content):
        prefix = ""
        if self.useTabs:
            for i in range(self.currentLevel):
                prefix = prefix + "\t"

        self.file.write(prefix + content + "\n")


    def goIn(self):
        self.currentLevel = self.currentLevel + 1


    def goOut(self):
        self.currentLevel = self.currentLevel - 1
        if self.currentLevel < 0:
            print("DEV ERROR: PEARRAY Exporter currentLevel < 0!")
