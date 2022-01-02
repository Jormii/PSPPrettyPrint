from enum import IntEnum

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

        class CharacterType(IntEnum):
            CHAR_TYPE_NORMAL = 0,
            CHAR_TYPE_NEW_LINE = 1,
            CHAR_TYPE_TAB = 2,
            CHAR_TYPE_RETURN_CARRIAGE = 3,
            CHAR_TYPE_WHITESPACE = 4

        def __init__(self, character, width, height, character_type=CharacterType.CHAR_TYPE_NORMAL):
            self.character = character
            self.unicode = ord(character)
            self.unicode_hex = hex(self.unicode)
            self.width = width
            self.height = height
            self.character_type = character_type

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
        self._write_font_set_header_file()
        self._write_font_header_source_file()

    def _write_font_set_header_file(self):
        font_header_template = """
        #ifndef {name_up}_FONT_H
        #define {name_up}_FONT_H

        #include "font.h"

        const Character *get_{name_low}_character(wchar_t unicode);

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

        const Character *get_{name_low}_character(wchar_t unicode)
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
        bitmap = bitmap[:, :character.width]

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
            .character_type={character_type},
            .width={width},
            .height={height},
            .bitmap=U_{unicode_hex}_{name_low}_bitmap}};""".format(
            unicode_hex=character.unicode_hex,
            name_low=self.name_low,
            character_type=character.character_type.name,
            width=character.width,
            height=character.height,
            bitmap=bitmap_array_str
        )

        # Switch case
        switch_case_template = """
        case {unicode_hex}:
            c = &U_{unicode_hex}_{name_low};
            break;""".format(
            unicode_hex=character.unicode_hex,
            name_low=self.name_low
        )

        # Update
        definitions += definition_template
        if index == self.unknown_character_index:
            default = "&U_{unicode_hex}_{name_low}".format(
                unicode_hex=character.unicode_hex,
                name_low=self.name_low)
        else:
            switch_cases += switch_case_template

        return definitions, default, switch_cases


if __name__ == "__main__":
    width = 7
    common_w = 5
    height = 10

    atlas = FontCreation.Atlas("./CharacterSet.png", width)
    characters = [
        # Unknown character
        FontCreation.Character("ô", width, height),

        # Control characters
        FontCreation.Character("\0", width, height),
        FontCreation.Character(
            "\t", width, height, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_TAB),
        FontCreation.Character(
            "\n", width, height, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_NEW_LINE),
        FontCreation.Character(
            "\r", width, height, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_RETURN_CARRIAGE),

        # Numbers
        FontCreation.Character("0", common_w, height),
        FontCreation.Character("1", 3, height),
        FontCreation.Character("2", common_w, height),
        FontCreation.Character("3", common_w, height),
        FontCreation.Character("4", common_w, height),
        FontCreation.Character("5", common_w, height),
        FontCreation.Character("6", common_w, height),
        FontCreation.Character("7", common_w, height),
        FontCreation.Character("8", common_w, height),
        FontCreation.Character("9", common_w, height),

        # Lower case
        FontCreation.Character("a", common_w, height),
        FontCreation.Character("b", common_w, height),
        FontCreation.Character("c", common_w, height),
        FontCreation.Character("d", common_w, height),
        FontCreation.Character("e", common_w, height),
        FontCreation.Character("f", 4, height),
        FontCreation.Character("g", common_w, height),
        FontCreation.Character("h", 4, height),
        FontCreation.Character("i", 2, height),
        FontCreation.Character("j", 4, height),
        FontCreation.Character("k", 4, height),
        FontCreation.Character("l", 2, height),
        FontCreation.Character("m", common_w, height),
        FontCreation.Character("n", 4, height),
        FontCreation.Character("o", common_w, height),
        FontCreation.Character("p", common_w, height),
        FontCreation.Character("q", common_w, height),
        FontCreation.Character("r", common_w, height),
        FontCreation.Character("s", common_w, height),
        FontCreation.Character("t", 4, height),
        FontCreation.Character("u", 4, height),
        FontCreation.Character("v", common_w, height),
        FontCreation.Character("w", common_w, height),
        FontCreation.Character("x", 4, height),
        FontCreation.Character("y", 4, height),
        FontCreation.Character("z", 4, height),

        # Upper case
        FontCreation.Character("A", common_w, height),
        FontCreation.Character("B", common_w, height),
        FontCreation.Character("C", common_w, height),
        FontCreation.Character("D", common_w, height),
        FontCreation.Character("E", common_w, height),
        FontCreation.Character("F", common_w, height),
        FontCreation.Character("G", common_w, height),
        FontCreation.Character("H", common_w, height),
        FontCreation.Character("I", 3, height),
        FontCreation.Character("J", common_w, height),
        FontCreation.Character("K", common_w, height),
        FontCreation.Character("L", common_w, height),
        FontCreation.Character("M", common_w, height),
        FontCreation.Character("N", common_w, height),
        FontCreation.Character("O", common_w, height),
        FontCreation.Character("P", common_w, height),
        FontCreation.Character("Q", common_w, height),
        FontCreation.Character("R", common_w, height),
        FontCreation.Character("S", common_w, height),
        FontCreation.Character("T", common_w, height),
        FontCreation.Character("U", common_w, height),
        FontCreation.Character("V", common_w, height),
        FontCreation.Character("W", common_w, height),
        FontCreation.Character("X", common_w, height),
        FontCreation.Character("Y", common_w, height),
        FontCreation.Character("Z", 4, height),

        # Symbols
        FontCreation.Character(
            chr(32), common_w, height, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_WHITESPACE),
        FontCreation.Character("!", 3, height),
        FontCreation.Character("\"", common_w, height),
        FontCreation.Character("#", common_w, height),
        FontCreation.Character("$", 4, height),
        FontCreation.Character("%", 5, height),
        FontCreation.Character("&", common_w, height),
        FontCreation.Character("'", 2, height),
        FontCreation.Character("(", 2, height),
        FontCreation.Character(")", 2, height),
        FontCreation.Character("*", common_w, height),
        FontCreation.Character("+", common_w, height),
        FontCreation.Character(",", 2, height),
        FontCreation.Character("-", common_w, height),
        FontCreation.Character(".", 2, height),
        FontCreation.Character("/", common_w, height),
        FontCreation.Character(":", 2, height),
        FontCreation.Character(";", 2, height),
        FontCreation.Character("<", 4, height),
        FontCreation.Character("=", common_w, height),
        FontCreation.Character(">", 4, height),
        FontCreation.Character("?", common_w, height),
        FontCreation.Character("@", common_w, height),
        FontCreation.Character("[", 3, height),
        FontCreation.Character("\\", common_w, height),
        FontCreation.Character("]", 3, height),
        FontCreation.Character("^", common_w, height),
        FontCreation.Character("_", common_w, height),
        FontCreation.Character("`", 2, height),
        FontCreation.Character("{", 4, height),
        FontCreation.Character("|", 1, height),
        FontCreation.Character("}", 4, height),
        FontCreation.Character("~", 4, height),

        # Caracteres españoles
        FontCreation.Character("á", common_w, height),
        FontCreation.Character("é", common_w, height),
        FontCreation.Character("í", 2, height),
        FontCreation.Character("ñ", 4, height),
        FontCreation.Character("ó", common_w, height),
        FontCreation.Character("ú", 4, height),
        FontCreation.Character("ü", 4, height),
        FontCreation.Character("Á", common_w, height),
        FontCreation.Character("É", common_w, height),
        FontCreation.Character("Í", 3, height),
        FontCreation.Character("Ñ", common_w, height),
        FontCreation.Character("Ó", common_w, height),
        FontCreation.Character("Ú", common_w, height),
        FontCreation.Character("Ü", common_w, height),
        FontCreation.Character("¡", 3, height),
        FontCreation.Character("¿", common_w, height)
    ]

    font_creation = FontCreation("base_character_set", atlas, characters, 0)
    font_creation.create_font()
