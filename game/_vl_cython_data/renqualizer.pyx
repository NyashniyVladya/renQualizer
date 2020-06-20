# -*- coding: utf-8 -*-
"""
@author: Vladya
"""

import pygame_sdl2
import renpy.color
import io
from os import path
from libc.stdint cimport uint32_t, uint64_t
from pygame_sdl2 cimport *

cdef int import_errors = import_pygame_sdl2()
if import_errors:
    raise ImportError("An error occurred while importing the pygame module.")

ctypedef uint32_t DWORD
ctypedef uint64_t QWORD

cdef extern from "bassMusicFileReader.c":

    int init()
    int freeStream()

    int setFile(const SDL_RWops *fileObject)
    int updateBufferFromPos(const int posInMs)
    double getInterpolatedPointInDB(
        const double xInOffsetFromA4,
        const double volume
    )


class BassDataHandler(object):

    __author__ = "Vladya"

    def __init__(self, renpy_audio_module):

        """
        :renpy_audio_module:
            'renpy.audio.audio' module object.
        """

        self.__current_file = None

        self.__audio_module = renpy_audio_module
        self.__renpysound = renpy_audio_module.renpysound

        self._init()

    def draw_equalizer(
        self,
        renpy_channel_or_data_tuple,
        width,
        height,
        line_color="#fff",
        back_color="#0000"
    ):

        """
        Draws an equalizer in black and white.
        (For easy subsequent repainting in the desired shades).
        """

        cdef int i
        cdef unsigned char white[4], black[4], v
        for i, v in enumerate(renpy.color.Color(line_color)):
            white[i] = v
        for i, v in enumerate(renpy.color.Color(back_color)):
            black[i] = v

        if isinstance(renpy_channel_or_data_tuple, tuple):
            data = renpy_channel_or_data_tuple
        else:
            data = self.get_data(renpy_channel_or_data_tuple)

        width = (int((float(width) / len(data))) * len(data)) # Align width.
        py_surface = pygame_sdl2.Surface((width, height), pygame_sdl2.SRCALPHA)
        cdef:
            SDL_Surface *c_surface = PySurface_AsSurface(py_surface)
            unsigned char *pixels = <unsigned char *> c_surface.pixels

        cdef:
            int elem_w = ((<int> ((<float> c_surface.w) / len(data))) or 1)
            int elem_h = c_surface.h

        cdef:
            int _x, x, y, element_index, x_offset, draw_from_y
            float value
            unsigned char *pixel_line, *current_color
        for element_index, value in enumerate(data):
            x_offset = elem_w * element_index
            draw_from_y = <int> (elem_h - ((<float> elem_h) * value))
            for y from 0 <= y < elem_h:
                pixel_line = (pixels + (c_surface.pitch * y))
                if y >= draw_from_y:
                    current_color = white
                else:
                    current_color = black
                for x from 0 <= x < elem_w:
                    _x = (x + x_offset) * 4
                    pixel_line[(_x + 0)] = current_color[0]
                    pixel_line[(_x + 1)] = current_color[1]
                    pixel_line[(_x + 2)] = current_color[2]
                    pixel_line[(_x + 3)] = current_color[3]
        return py_surface

    @classmethod
    def __get_exception(cls, action=None, error_code=None, _exc_type=None):
        ExceptionType = Exception
        if hasattr(_exc_type, "__mro__"):
            if Exception in _exc_type.__mro__:
                ExceptionType = _exc_type
        trace_text = "{0}:".format(cls.__name__)
        if action:
            trace_text += " An error occurred while {0}.".format(action)
        if error_code:
            trace_text += " Error code is '{0}'.".format(error_code)
        trace_text = trace_text.rstrip(':')
        return ExceptionType(trace_text)

    def _init(self):
        cdef int error_code = init()
        if error_code == (-2):
            # Already initialized.
            pass
        elif error_code:
           raise self.__get_exception("initialize recording", error_code)

    def free_resources(self):
        cdef int error_code = freeStream()
        if error_code == (-2):
            # Not initialized.
            pass
        elif error_code:
           raise self.__get_exception("freeing resources", error_code)

    def get_volume(self, renpy_channel):
        return max(self.get_data(renpy_channel))

    def get_data(self, renpy_channel):
        result = self._get_data(renpy_channel)
        if result:
            return result
        return tuple(self._get_octave_mapping(fill_zeros=True))

    def _get_data(self, renpy_channel):

        """
        :renpy_channel:
            'renpy.audio.audio.Channel' object.
        Return tuple with FFT octaved data or None.
        """

        cdef:
            int pos_in_ms
            int error_code
            float volume
            SDL_RWops *_c_file

        with self.__audio_module.lock:

            if not self.__audio_module.pcm_ok:
                return None

            renpy_path = self.__renpysound.playing_name(renpy_channel.number)
            if not renpy_path:
                return None

            pos_in_ms = self.__renpysound.get_pos(renpy_channel.number)
            if pos_in_ms == (-1):
                return None

            volume = self.__renpysound.get_volume(renpy_channel.number)

            is_audiodata = False
            if hasattr(self.__audio_module, "AudioData"):
                if isinstance(renpy_path, self.__audio_module.AudioData):
                    is_audiodata = True

            if not is_audiodata:
                renpy_path = u"".join(
                    (
                        renpy_channel.file_prefix,
                        renpy_channel.split_filename(renpy_path, False)[0],
                        renpy_channel.file_suffix
                    )
                )
                renpy_path = path.normpath(renpy_path).replace(u'\\', u'/')

            if self.__current_file != renpy_path:

                if is_audiodata:
                    file_func = io.BytesIO
                    _arg = renpy_path.data
                else:
                    file_func = self.__audio_module.load
                    _arg = renpy_path

                with file_func(_arg) as _py_file:

                    _c_file = RWopsFromPython(_py_file)
                    if not _c_file:
                        raise self.__get_exception("read data")
                    error_code = setFile(_c_file)
                    _c_file.close(_c_file)

                if error_code == (-2):
                    raise self.__get_exception(
                        "setting file (channel has not been initialized)"
                    )
                elif error_code == (-4):
                   raise self.__get_exception(
                       "allocating memory",
                       _exc_type=MemoryError
                   )
                elif error_code:
                    # Another error.
                    raise self.__get_exception("setting file", error_code)

                self.__current_file = renpy_path

            error_code = updateBufferFromPos(pos_in_ms)
            if error_code == (-2):
                raise self.__get_exception(
                    "updating buffer (channel has not been initialized)"
                )
            elif error_code == (-4):
                raise self.__get_exception(
                    "allocating memory",
                    _exc_type=MemoryError
                )
            elif error_code == 7:  # 'BASS_ERROR_POSITION'. File is end.
                return None
            elif error_code:
                # Another error.
                raise self.__get_exception("updating buffer", error_code)

            return tuple(self._get_octave_mapping(volume=volume))

    def _get_octave_mapping(self, volume=1., fill_zeros=False):

        """
        Yield tuple with FFT octaved data.
        """

        cdef:
            int i
            double hz, value

        for i from (-45) <= i < 63:  # From C1 to B9 in semitones.

            if fill_zeros:
                yield .0
                continue

            value = getInterpolatedPointInDB((<double> i), volume)
            if value == (-1.):
                raise self.__get_exception("generated octaved mapping")
            yield value
