from enum import IntEnum

import matplotlib.pyplot as plt

FONT_HEADER_TEMPLATE_PATH = "./templates/font_header.txt"
FONT_HEADER_TEMPLATE = open(FONT_HEADER_TEMPLATE_PATH, "r").read()

FONT_SOURCE_TEMPLATE_PATH = "./templates/font_source.txt"
FONT_SOURCE_TEMPLATE = open(FONT_SOURCE_TEMPLATE_PATH, "r").read()

FONT_CHARACTER_DEFINITION_TEMPLATE_PATH = "./templates/character_definition.txt"
FONT_CHARACTER_DEFINITION_TEMPLATE = open(
    FONT_CHARACTER_DEFINITION_TEMPLATE_PATH, "r").read()

FONT_CHARACTER_SWITCH_CASE_TEMPLATE_PATH = "./templates/switch_case.txt"
FONT_CHARACTER_SWITCH_CASE_TEMPLATE = open(
    FONT_CHARACTER_SWITCH_CASE_TEMPLATE_PATH, "r").read()


class FontCreation:

    class Atlas:

        def __init__(self, path, character_width, character_height):
            image = plt.imread(path)
            self.atlas = image[:, :, 0]
            self.w = self.atlas.shape[1]
            self.h = self.atlas.shape[0]

            self.character_width = character_width
            self.character_height = character_height
            self.characters_stored = self.w // character_width

            if (self.w % character_width) != 0:
                exit("Atlas width ({}) isn't a multiple of input width ({})".format(
                    self.w, character_width))

            if self.h != character_height:
                exit("Atlas height ({}) isn't the same as input height ({})".format(
                    self.h, self.character_height))

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
            CHAR_TYPE_WHITESPACE = 4,
            CHAR_TYPE_NULL = 5

        def __init__(self, character, width, character_type=CharacterType.CHAR_TYPE_NORMAL):
            self.character = character
            self.code_point = ord(character)
            self.code_point_hex = hex(self.code_point)
            self.width = width
            self.character_type = character_type

        def __repr__(self):
            return "{} ({} / {}). w = {}".format(
                self.character, self.code_point, self.code_point_hex, self.width)

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

        code_point_set = set()
        for character in characters:
            if character.code_point in code_point_set:
                exit("Duplicated character with code_point {}".format(
                    character.code_point_hex))

            if character.width > atlas.character_width:
                exit("Character {} is wider than input atlas ({})".format(
                    character, atlas.character_width))

            code_point_set.add(character.code_point)

    def create_font(self):
        self._write_font_set_header_file()
        self._write_font_header_source_file()

    def _write_font_set_header_file(self):
        header_content = FONT_HEADER_TEMPLATE.format(
            name_up=self.name_up,
            name_low=self.name_low,
        )

        header_path = "./{}_font.h".format(self.name_low)
        file = open(header_path, "w")
        file.write(header_content)
        file.close()

    def _write_font_header_source_file(self):
        source_content = FONT_SOURCE_TEMPLATE.format(
            name_low=self.name_low,
            height=self.atlas.character_height,
            characters_definitions=self._source_characters_definitions(),
            default=self._source_default(),
            switch_cases=self._source_switch_cases()
        )

        source_path = "./{}_font.c".format(self.name_low)
        file = open(source_path, "w")
        file.write(source_content)
        file.close()

    def _source_characters_definitions(self):
        definitions = ""
        for index, character in enumerate(self.characters):
            bitmap = self.atlas.get(index)
            bitmap = bitmap[:, :character.width]

            flatten = bitmap.flatten()
            bitmap_array_str = "{"
            for k in range(len(flatten)):
                bitmap_array_str += "1" if flatten[k] else "0"
                if (k + 1) != len(flatten):
                    bitmap_array_str += ", "

            bitmap_array_str += "}"

            character_definition = FONT_CHARACTER_DEFINITION_TEMPLATE.format(
                code_point_hex=character.code_point_hex,
                name_low=self.name_low,
                character_type=character.character_type.name,
                width=character.width,
                height=self.atlas.character_height,
                bitmap=bitmap_array_str
            )
            definitions += character_definition

        return definitions

    def _source_default(self):
        if self.unknown_character_index is None:
            return "0"
        else:
            unknown_character = self.characters[self.unknown_character_index]
            return "&U_{code_point_hex}_{name_low}".format(
                code_point_hex=unknown_character.code_point_hex,
                name_low=self.name_low
            )

    def _source_switch_cases(self):
        switch_cases = ""
        for index, character in enumerate(self.characters):
            if index == self.unknown_character_index:
                continue

            switch_case = FONT_CHARACTER_SWITCH_CASE_TEMPLATE.format(
                code_point_hex=character.code_point_hex,
                name_low=self.name_low
            )
            switch_cases += switch_case

        return switch_cases


if __name__ == "__main__":
    width = 7
    common_w = 5
    height = 10

    atlas = FontCreation.Atlas("./CharacterSet.png", width, height)
    characters = [
        # Unknown character
        FontCreation.Character("ô", width),

        # Control characters
        FontCreation.Character(
            "\0", width, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_NULL),
        FontCreation.Character(
            "\t", width, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_TAB),
        FontCreation.Character(
            "\n", width, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_NEW_LINE),
        FontCreation.Character(
            "\r", width, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_RETURN_CARRIAGE),

        # Numbers
        FontCreation.Character("0", common_w),
        FontCreation.Character("1", 3),
        FontCreation.Character("2", common_w),
        FontCreation.Character("3", common_w),
        FontCreation.Character("4", common_w),
        FontCreation.Character("5", common_w),
        FontCreation.Character("6", common_w),
        FontCreation.Character("7", common_w),
        FontCreation.Character("8", common_w),
        FontCreation.Character("9", common_w),

        # Lower case
        FontCreation.Character("a", common_w),
        FontCreation.Character("b", common_w),
        FontCreation.Character("c", common_w),
        FontCreation.Character("d", common_w),
        FontCreation.Character("e", common_w),
        FontCreation.Character("f", 4),
        FontCreation.Character("g", common_w),
        FontCreation.Character("h", 4),
        FontCreation.Character("i", 2),
        FontCreation.Character("j", 4),
        FontCreation.Character("k", 4),
        FontCreation.Character("l", 2),
        FontCreation.Character("m", common_w),
        FontCreation.Character("n", 4),
        FontCreation.Character("o", common_w),
        FontCreation.Character("p", common_w),
        FontCreation.Character("q", common_w),
        FontCreation.Character("r", common_w),
        FontCreation.Character("s", common_w),
        FontCreation.Character("t", 4),
        FontCreation.Character("u", 4),
        FontCreation.Character("v", common_w),
        FontCreation.Character("w", common_w),
        FontCreation.Character("x", 4),
        FontCreation.Character("y", 4),
        FontCreation.Character("z", 4),

        # Upper case
        FontCreation.Character("A", common_w),
        FontCreation.Character("B", common_w),
        FontCreation.Character("C", common_w),
        FontCreation.Character("D", common_w),
        FontCreation.Character("E", common_w),
        FontCreation.Character("F", common_w),
        FontCreation.Character("G", common_w),
        FontCreation.Character("H", common_w),
        FontCreation.Character("I", 3),
        FontCreation.Character("J", common_w),
        FontCreation.Character("K", common_w),
        FontCreation.Character("L", common_w),
        FontCreation.Character("M", common_w),
        FontCreation.Character("N", common_w),
        FontCreation.Character("O", common_w),
        FontCreation.Character("P", common_w),
        FontCreation.Character("Q", common_w),
        FontCreation.Character("R", common_w),
        FontCreation.Character("S", common_w),
        FontCreation.Character("T", common_w),
        FontCreation.Character("U", common_w),
        FontCreation.Character("V", common_w),
        FontCreation.Character("W", common_w),
        FontCreation.Character("X", common_w),
        FontCreation.Character("Y", common_w),
        FontCreation.Character("Z", 4),

        # Symbols
        FontCreation.Character(
            chr(32), common_w, character_type=FontCreation.Character.CharacterType.CHAR_TYPE_WHITESPACE),
        FontCreation.Character("!", 3),
        FontCreation.Character("\"", common_w),
        FontCreation.Character("#", common_w),
        FontCreation.Character("$", 4),
        FontCreation.Character("%", 5),
        FontCreation.Character("&", common_w),
        FontCreation.Character("'", 2),
        FontCreation.Character("(", 2),
        FontCreation.Character(")", 2),
        FontCreation.Character("*", common_w),
        FontCreation.Character("+", common_w),
        FontCreation.Character(",", 2),
        FontCreation.Character("-", common_w),
        FontCreation.Character(".", 2),
        FontCreation.Character("/", common_w),
        FontCreation.Character(":", 2),
        FontCreation.Character(";", 2),
        FontCreation.Character("<", 4),
        FontCreation.Character("=", common_w),
        FontCreation.Character(">", 4),
        FontCreation.Character("?", common_w),
        FontCreation.Character("@", common_w),
        FontCreation.Character("[", 3),
        FontCreation.Character("\\", common_w),
        FontCreation.Character("]", 3),
        FontCreation.Character("^", common_w),
        FontCreation.Character("_", common_w),
        FontCreation.Character("`", 2),
        FontCreation.Character("{", 4),
        FontCreation.Character("|", 1),
        FontCreation.Character("}", 4),
        FontCreation.Character("~", 4),

        # Caracteres españoles
        FontCreation.Character("á", common_w),
        FontCreation.Character("é", common_w),
        FontCreation.Character("í", 2),
        FontCreation.Character("ñ", 4),
        FontCreation.Character("ó", common_w),
        FontCreation.Character("ú", 4),
        FontCreation.Character("ü", 4),
        FontCreation.Character("Á", common_w),
        FontCreation.Character("É", common_w),
        FontCreation.Character("Í", 3),
        FontCreation.Character("Ñ", common_w),
        FontCreation.Character("Ó", common_w),
        FontCreation.Character("Ú", common_w),
        FontCreation.Character("Ü", common_w),
        FontCreation.Character("¡", 3),
        FontCreation.Character("¿", common_w)
    ]

    font_creation = FontCreation("base_set", atlas, characters, 0)
    font_creation.create_font()
