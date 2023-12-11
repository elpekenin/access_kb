#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Automatically include on your compilation all QP assets on relevant paths.
Also provides a function to load them into memory at once.
"""

import sys
from functools import partial
from pathlib import Path
from typing import Callable

from scripts import *

OUTPUT_NAME = "qp_resources"

H_FILE = lines(
    H_HEADER,
    "",
    "{generated_code}"
)

C_FILE = lines(
    C_HEADER,
    "",
    f'#include "graphics.h"',
    "",
    "void load_qp_resources(void) {{",
        "{generated_code}"  # no comma here intentionally
    "}}"
)

MK_FILE = lines(
    MK_HEADER,
    "",
    "{generated_code}"
)

AssetsDictT = dict[str, list[Path]]

def _find_assets_impl(assets: AssetsDictT, path: str):
    folder = Path(path).absolute() / "painter"

    if not dir_exists(folder):
        return

    assets["fonts"].extend(list(folder.glob("fonts/*qff.h")))
    assets["images"].extend(list(folder.glob("images/*qgf.h")))


def _find_assets(paths: list[str]) -> AssetsDictT:
    assets = {"fonts": [], "images": []}

    for path in paths:
        _find_assets_impl(assets, path)

    return assets


def __for_all_assets(func: Callable, assets: AssetsDictT) -> str:
    return "\n".join(
        # sort assets by name
        func(asset_k, sorted(asset_v, key=lambda e: e.name))
        for asset_k, asset_v in assets.items()
    )


def _h_generator(key: str, paths: list[Path]) -> str:
    return lines(
        f"// {key}",
        "\n".join(
            f'#include "{path.name}"'
            for path in paths
        ),
        ""
    )


def _c_generator(key: str, paths: list[Path]) -> str:
    macro = "load_font" if key == "fonts" else "load_image"

    def _name_generator(key: str, path: Path) -> str:
        key = "font" if key == "fonts" else "gfx"
        name = (path.name
                .replace('-', '_')
                .replace('.qff.h', '')
                .replace('.qgf.h', ''))

        return f"{key}_{name}"

    return lines(
        f"    // {key}",
        "\n".join(
            f"    {macro}({_name_generator(key, path)});"
            for path in paths
        ),
        ""
    )

def _mk_generator(key: str, paths: list[Path]) -> str:
    return lines(
        f"# {key}",
        "\n".join(
            f"SRC += {path}".replace(".h", ".c")
            for path in paths
        ),
        ""
    )


if __name__ == "__main__":
    # -- Handle args
    if len(sys.argv) < 3: # executable, output path, paths
        print(f"{CLI_ERROR} {current_filename(__file__)} <paths...>")
        exit(1)

    output_dir = Path(sys.argv[1])
    if not dir_exists(output_dir):
        print(f"Invalid path {output_dir}")
        exit(1)

    # Find elements
    assets = _find_assets(sys.argv[2:])

    # Gen files
    _for_all_assets = partial(__for_all_assets, assets=assets)

    gen_h = _for_all_assets(_h_generator)
    with open(output_dir / f"{OUTPUT_NAME}.h", "w") as f:
        f.write(H_FILE.format(generated_code=gen_h))

    gen_c = _for_all_assets(_c_generator)
    with open(output_dir / f"{OUTPUT_NAME}.c", "w") as f:
        f.write(C_FILE.format(generated_code=gen_c))

    gen_mk = _for_all_assets(_mk_generator)
    with open(output_dir / f"{OUTPUT_NAME}.mk", "w") as f:
        f.write(MK_FILE.format(generated_code=gen_mk))

else:
    print("Dont try to import this")
    exit(1)
