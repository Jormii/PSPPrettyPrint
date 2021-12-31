import numpy as np
import matplotlib.pyplot as plt


class FontInput:

    class Character:

        def __init__(self, unicode, kerning=0):
            self.unicode = unicode
            self.kerning = kerning

    def __init__(self, name, char_w, char_h, atlas_path, characters):
        self.name = name
        self.char_w = char_w
        self.char_h = char_h
        self.image = plt.imread(atlas_path)
        self.characters = characters

        image_h = self.image.shape[0]
        image_w = self.image.shape[1]
        if (image_w % char_w) != 0 or (image_h % char_h) != 0:
            print("Any of the sides is not a multiple of input width or height. Dimensions: {}x{}, input: {}x{}".format(
                image_w, image_h, char_w, char_h))
            exit(1)

        self.amount = image_w // char_w
        if self.amount != len(characters):
            print(
                "Mismatch in number of characters in atlas and number of characters in input list")
            exit(1)

        characters_set = set()
        for i in range(self.amount):
            characters_set.add(characters[i].unicode)

        if len(characters) != len(characters_set):
            print("Any of the characters given is duplicated")
            exit(1)
