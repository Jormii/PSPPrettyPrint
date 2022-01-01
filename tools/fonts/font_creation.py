import numpy as np
import matplotlib.pyplot as plt


class FontCreation:

    class Atlas:

        def __init__(self, path, character_width):
            image = plt.imread(path)
            self.atlas = image[:, :, 0]
            self.w = self.atlas.shape[1]
            self.h = self.atlas.shape[0]
            self.character_width = character_width
            self.characters_stored = self.w // character_width

            if (self.w % character_width) != 0:
                exit("Atlas width ({}) isn't a multiple of input width ({})".format(
                    self.w, character_width))

        def get(self, index):
            if index < 0 or index > self.characters_stored:
                exit("Trying to get character given index {}. Atlas stores {}".format(
                    index, self.characters_stored))

            low = index * self.character_width
            high = (index + 1) * self.character_width
            return self.atlas[:, low: high]

    class Character:

        def __init__(self, character, width, height):
            self.character = character
            self.unicode = ord(character)
            self.unicode_hex = hex(self.unicode)
            self.width = width
            self.height = height

        def __repr__(self):
            return "{} ({} / {}). (w, h) = ({}, {})".format(
                self.character, self.unicode, self.unicode_hex, self.width, self.height)

    def __init__(self, name, atlas, characters, unknown_character_index=None):
        self.name = name
        self.name_up = name.upper()
        self.name_low = name.lower()
        self.atlas = atlas
        self.characters = characters
        self.unknown_character_index = unknown_character_index

        # Checking if parameters are congruent
        if atlas.characters_stored != len(characters):
            exit("Mismatch in characters stored in atlas ({}) and the length of input characters ({})".format(
                atlas.characters_stored, len(characters)))

        unicode_set = set()
        for character in characters:
            if character.unicode in unicode_set:
                exit("Duplicated character with unicode {}".format(
                    character.unicode_hex))

            if character.width > atlas.character_width:
                exit("Character {} is wider than input atlas ({})".format(
                    character, atlas.character_width))

            if character.height > atlas.h:
                exit("Character {} is taller than input atlas ({})".format(
                    character.height, atlas.h))

            unicode_set.add(character.unicode)

    def create_font(self):
        self._create_character_header_file()
        self._write_font_set_header_file()
        self._write_font_header_source_file()

    def _create_character_header_file(self):
        character_header_template = """
        #ifndef CHARACTER_H
        #define CHARACTER_H

        #include <stdint.h>

        typedef struct Character_st
        {
            uint8_t flags;
            uint8_t width;
            uint8_t height;
            uint8_t *bitmap;
        } Character;

        #endif"""

        file = open("./character.h", "w")
        file.write(character_header_template)
        file.close()

    def _write_font_set_header_file(self):
        font_header_template = """
        #ifndef {name_up}_FONT_H
        #define {name_up}_FONT_H

        #include <stdint.h>

        #include "character.h"

        const Character *get_{name_low}_character(uint32_t unicode);

        #endif""".format(
            name_up=self.name_up,
            name_low=self.name_low
        )

        header_path = "./{}_font.h".format(self.name_low)
        file = open(header_path, "w")
        file.write(font_header_template)
        file.close()

    def _write_font_header_source_file(self):
        definitions, default, switch_cases = self._format_source_file()

        font_source_template = """#include "{name_low}_font.h"
        {definitions}

        const Character *get_{name_low}_character(uint32_t unicode)
        {{
            Character *c = {default};
            switch (unicode) {{{switch_cases}
                default:
                    break;
            }}

            return c;
        }}""".format(
            name_low=self.name_low,
            definitions=definitions,
            default=default,
            switch_cases=switch_cases
        )

        source_path = "./{}_font.c".format(self.name_low)
        file = open(source_path, "w")
        file.write(font_source_template)
        file.close()

    def _format_source_file(self):
        definitions = ""
        switch_cases = ""
        default = "0"

        for i in range(len(self.characters)):
            definitions, default, switch_cases = self._format_character(
                i, definitions, switch_cases, default)

            if (i + 1) != len(self.characters):
                definitions += "\n"

        return definitions, default, switch_cases

    def _format_character(self, index, definitions, switch_cases, default):
        character = self.characters[index]

        # Definition
        bitmap = self.atlas.get(index)
        flatten = bitmap.flatten()
        bitmap_array_str = "{"
        for i in range(len(flatten)):
            bitmap_array_str += "1" if flatten[i] else "0"
            if (i + 1) != len(flatten):
                bitmap_array_str += ", "

        bitmap_array_str += "}"

        definition_template = """
        uint8_t U_{unicode_hex}_{name_low}_bitmap[{width}*{height}] = {bitmap};
        Character U_{unicode_hex}_{name_low} = {{
            .flags=0,
            .width={width},
            .height={height},
            .bitmap=U_{unicode_hex}_{name_low}_bitmap}};""".format(
            unicode_hex=character.unicode_hex,
            name_low=self.name_low,
            width=character.width,
            height=character.height,
            bitmap=bitmap_array_str
        )

        # Switch case
        switch_case_template = """
        case ({unicode_hex}):
            c = &U_{unicode_hex}_{name_low};
            break;""".format(
            unicode_hex=character.unicode_hex,
            name_low=self.name_low
        )

        # Update
        definitions += definition_template
        switch_cases += switch_case_template
        if index == self.unknown_character_index:
            default = "&U_{unicode_hex}_{name_low};".format(
                unicode_hex=character.unicode_hex,
                name_low=self.name_low)

        return definitions, default, switch_cases


if __name__ == "__main__":
    width = 7
    height = 10

    atlas = FontCreation.Atlas("./CharacterSet.png", width)
    characters = [
        # Control characters
        FontCreation.Character("\0", width, height),
        FontCreation.Character("\t", width, height),
        FontCreation.Character("\n", width, height),
        FontCreation.Character("\r", width, height)
    ]

    font_creation = FontCreation("control_characters", atlas, characters)
    font_creation.create_font()
