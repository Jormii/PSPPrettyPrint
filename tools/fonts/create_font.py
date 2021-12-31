import numpy as np
import matplotlib.pyplot as plt

from font_input import FontInput
from font_output import FontOutput


def create_font(font_input: FontInput):
    rgb = font_input.image[:, :, 0:3]
    bw = np.sum(rgb, axis=2).astype(bool)

    out = FontOutput(font_input.name)
    for i in range(font_input.amount):
        ith_char = bw[:, i*font_input.char_w: (i+1)*font_input.char_w]
        flattened = ith_char.flatten()

        out_char = FontOutput.Character(
            font_input.characters[i].unicode, flattened)
        out.characters.append(out_char)

    return out


def write_header_file(font_output: FontOutput):
    header_format = """
    # ifndef {name_up}_FONT_H
    # define {name_up}_FONT_H

    # include <stdint.h>

    uint8_t *get_{name_low}_character(uint32_t unicode);

    # endif
    """.format(name_low=font_output.name.lower(), name_up=font_output.name.upper())

    header_path = "./{}_font.h".format(font_output.name.lower())
    file = open(header_path, "w")
    file.write(header_format)
    file.close()

def write_source_file(font_output: FontOutput):
    definitions = ""
    switch_cases = ""
    for i in range(len(font_output.characters)):
        unicode = font_output.characters[i].unicode
        array = font_output.characters[i].array

        # Definition
        length = len(array)
        definition = "uint8_t U_{}_{}[{}] = {{{}, ".format(
            font_output.name.lower(), hex(unicode), length + 1, length)
        for j in range(length):
            definition += "1" if array[i] else "0"
            if (j + 1) != length:
                definition += ", "

        definitions += "{}}};".format(definition)

        # Switch case
        case_format = """case({}):
            character = U_{}_{};
            break;""".format(hex(unicode), font_output.name.lower(), hex(unicode))
        switch_cases += case_format

        if (i + 1) != len(font_output.characters):
            definitions += "\n"
            switch_cases += "\n"

    definition += "}"

    source_format = """
    #include "{name_low}_font.h"
    
    {definitions}

    uint8_t *get_{name_low}_character(uint32_t unicode) {{
        uint8_t *character = 0;

        switch(unicode) {{
            {switch_cases}
            default:
                break;
        }}

        return character;
    }}
    """.format(name_low=font_output.name.lower(), definitions=definitions, switch_cases=switch_cases)

    source_path = "./{}_font.c".format(font_output.name.lower())
    file = open(source_path, "w")
    file.write(source_format)
    file.close()


if __name__ == "__main__":
    width = 5
    height = 7
    path = "./Numbers.png"
    characters = []
    for i in range(10):
        char = FontInput.Character(ord(str(i)))
        characters.append(char)

    font_input = FontInput("Numbers", width, height, path, characters)
    font_output = create_font(font_input)
    write_header_file(font_output)
    write_source_file(font_output)
