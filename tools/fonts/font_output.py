class FontOutput:

    class Character:

        def __init__(self, unicode, array):
            self.unicode = unicode
            self.array = array

    def __init__(self, name):
        self.name = name
        self.characters = []
