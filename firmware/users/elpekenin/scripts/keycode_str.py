#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Extract keycodes as strings from a keymap.c file

Assumes layers written as: [<layer>] = LAYOUT(.*)(
i.e. Layers with explicit numbering/naming
     Numbering and LAYOUT on the same line
"""

import re
import sys
from pathlib import Path

from scripts import *


LAYER = []
KEYMAP = []

COMMENT = re.compile("//(.*)")
MULTI_COMMENT = re.compile(r"/\*(.*?)\*/")
LAYOUT = re.compile(r"\[(.*)\]( *)=( *)LAYOUT(.*)\(")

OUTPUT_NAME = "generated_keycode_str"

COORD_TO_INDEX_ARRAY = "__col_row_to_index_mapping"
KEYCODE_ARRAY = "__keycodes"
GETTER_SIGNATURE = "const char *get_keycode_str_at(uint8_t layer, uint8_t row, uint8_t col)"
INDEX_TO_COORDS_SIGNATURE = "bool index_to_row_col(uint8_t index, uint8_t *row, uint8_t *col)"
COORDS_TO_INDEX_SIGNATURE =  "bool row_col_to_index(uint8_t row, uint8_t col, uint8_t *index)"

H_FILE = lines(
    H_HEADER,
    "",
    GETTER_SIGNATURE + ";",
    INDEX_TO_COORDS_SIGNATURE + ";",
    COORDS_TO_INDEX_SIGNATURE + ";",
)

C_FILE = lines(
    C_HEADER,
    "",
    '#include "default_keyboard.h" // for LAYOUT',
    '#include "user_layers.h" // for layer names',
    "",
    "{coord2index_array}",
    "",
    "{keycode_array}",
    "",
    "{getter_func}",
    "",
    "{index2coord}",
    "",
    "{coord2index}"
)


def _read_file(keymap_path: Path) -> str:
    with open(
        keymap_path,
        "r"
    ) as f:
        return f.read()


def _remove_comments(raw_file: str) -> str:
    clean = raw_file

    while res := COMMENT.search(clean):
        start, end = res.span()
        clean = clean[:start] + clean[end:]

    while res := MULTI_COMMENT.search(clean):
        start, end = res.span()
        clean = clean[:start] + clean[end:]

    return clean


def _extract_layout_name(clean_file: str) -> str:
    suffix = LAYOUT.search(clean_file).groups()[-1]
    return f"LAYOUT{suffix}(" 


def _extract_layer_names(clean_file: str) -> list[str]:
    return [match[0] for match in LAYOUT.findall(clean_file)]


def _extract_keycodes(clean_file: str) -> list[str]:
    accumulated = ""
    layers = []
    parens = 0

    layout = LAYOUT.search(clean_file)
    start = layout.start()

    for char in clean_file[start:]:
        if char == "(":
            parens += 1

        elif char == ")":
            parens -= 1

        if parens and not (parens == 1 and char == "("):
            accumulated += char

        if parens == 0:  # outsize of layout macro
            if char == "}": # keymap array end
                break

            if accumulated:  # if gathered a layer of keycodes, store it
                accumulated = ", ".join([i.strip() for i in accumulated.replace("\n", "").split(", ")])
                layers.append(accumulated)
                accumulated = ""

    return layers


def _coord2index_array_generator(layers: list[str], layout_name: str) -> str:
    return (
        f"static const uint8_t {COORD_TO_INDEX_ARRAY}[MATRIX_ROWS][MATRIX_COLS] = {layout_name}"
        f'{", ".join(str(i+1) for i in range(len(layers[0].split(","))))}'
        ");"
    )


def _keycode_generator(layers: list[str], layer_names: list[str]) -> str:
    def _string(kc: str) -> str:
        """Helper to pretty format the keycodes."""
        return f'"{kc.strip()}"'.rjust(10)

    strings = f"static const char* {KEYCODE_ARRAY}[][MATRIX_ROWS][MATRIX_COLS] = {{\n"
    for name, layer in zip(layer_names, layers):
        name = f"[{name}]".rjust(15)
        names = ", ".join(_string(keycode) for keycode in layer.split(","))
        strings += f"{name} = LAYOUT({names}),\n"
    strings += "};"

    return strings


def _getter_generator() -> str:
    return lines(
        f"{GETTER_SIGNATURE} {{",
        f"    return {KEYCODE_ARRAY}[layer][row][col];",
        "}"
    )

def _index2coord_generator() -> str:
    return lines(
        f"{INDEX_TO_COORDS_SIGNATURE} {{",
        "    // we need this correction as the counting on map is not 0-based",
        "    // it is 1-based and 0 represents no key on the position",
        "    uint8_t corrected_index = index + 1;",
        "",
        "    for (uint8_t i = 0; i < MATRIX_ROWS; ++i) {",
        "        for (uint8_t j = 0; j < MATRIX_COLS; ++j) {",
        f"            if ({COORD_TO_INDEX_ARRAY}[i][j] == corrected_index) {{",
        "                *row = i;",
        "                *col = j;",
        "                return true;",
        "            }",
        "        }",
        "    }",
        "",
        "    return false;",
        "}",
    )


def _coord2index_generator() -> str:
    return lines(
        f"{COORDS_TO_INDEX_SIGNATURE} {{",
        f"    uint8_t i = {COORD_TO_INDEX_ARRAY}[row][col];",
        "",
        "    if (i != KC_NO) {",
        "        *index = i - 1;",
        "    }",
        "",
        "    return i != KC_NO;",
        "}",
    )


if __name__ == "__main__":
    # -- Handle args
    if len(sys.argv) < 3: # executable, output path, keymap path
        print(f"{CLI_ERROR} {current_filename(__file__)} <output_path> <keymap_path>")
        exit(1)

    output_dir = Path(sys.argv[1])
    if not dir_exists(output_dir):
        print(f"Invalid path {output_dir}")
        exit(1)

    keymap_file = Path(sys.argv[2]) / "keymap.c"
    if not file_exists(keymap_file):
        print(f"Invalid keymap {keymap_file}")
        exit(1)

    # parse file
    raw = _read_file(keymap_file)
    clean = _remove_comments(raw)
    layout_name = _extract_layout_name(clean)
    layer_names = _extract_layer_names(clean)
    layers = _extract_keycodes(clean)

    # generate file
    coord2index_array = _coord2index_array_generator(layers, layout_name)
    keycode_array = _keycode_generator(layers, layer_names)
    getter_func = _getter_generator()
    index2coord = _index2coord_generator()
    coord2index = _coord2index_generator()

    with open(output_dir / f"{OUTPUT_NAME}.h", "w") as f:
        f.write(H_FILE)

    with open(output_dir / f"{OUTPUT_NAME}.c", "w") as f:
        f.write(
            C_FILE.format(
                coord2index_array=coord2index_array,
                keycode_array=keycode_array,
                getter_func=getter_func,
                index2coord=index2coord,
                coord2index=coord2index,
            )
        )

else:
    print("Dont try to import this")
    exit(1)