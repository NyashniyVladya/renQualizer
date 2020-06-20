# -*- coding: utf-8 -*-
"""
@author: Vladya
"""

import sys
import distutils
from os import path
from setuptools import setup
from Cython.Build import cythonize

renpy_modules = path.abspath("C:\\CLibs\\RenPyC\\include")
if renpy_modules not in sys.path:
    sys.path.insert(0, renpy_modules)


ADDITIONS = {
    "include_dirs": [
        "C:\\MinGW\\include",  # MinGW default
        "C:\\CLibs\\SDL2_MSVC\\include",
        "C:\\CLibs\\Bass"
    ],
    "libraries": [
        "bass",  # Основная либа bass.
    ],
    "library_dirs": [
        "C:\\CLibs\\Bass"
    ],
    "runtime_library_dirs": [
        "C:\\CLibs\\Bass",
    ],
    "define_macros": [
    ]
}


if __name__ == "__main__":

    ext = distutils.core.Extension(
        name='renqualizer',
        sources=["*.pyx"],
        include_dirs=ADDITIONS["include_dirs"],
        library_dirs=ADDITIONS["library_dirs"],
        libraries=ADDITIONS["libraries"],
        runtime_library_dirs=ADDITIONS["runtime_library_dirs"],
        define_macros=ADDITIONS["define_macros"]
    )
    setup(ext_modules=cythonize([ext]))
