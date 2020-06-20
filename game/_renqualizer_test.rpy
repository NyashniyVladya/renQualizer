
init -10 python in _renqualizer_test:

    import renqualizer  # cython
    from store import NoRollback, Transform

    _bass_object = renqualizer.BassDataHandler(
        renpy_audio_module=renpy.audio.audio
    )

    class Equalizer(renpy.Displayable, NoRollback):

        __author__ = "Vladya"

        def __init__(
            self,
            width=None,
            height=None,
            line_color="#fff",
            back_color="#0000",
            channel="music"
        ):
            super(Equalizer, self).__init__()
            self._channel = renpy.audio.audio.get_channel(channel)
            self.width = width
            self.height = height

            self._colors = (line_color, back_color)
            self.__last_data = None
            self.__last_surf = None
            self.__st = None

        def __eq__(self, other):
            if not self._equals(other):
                return False
            if self._channel != other._channel:
                return False
            if (self.width, self.height) != (other.width, other.height):
                return False
            if self._colors != other._colors:
                return False
            return True

        def __ne__(self, other):
            return (not self.__eq__(other))

        @property
        def _colors(self):
            return (self.__line_color, self.__back_color)

        @_colors.setter
        def _colors(self, new_colors):
            line_color, back_color = new_colors
            line_color = renpy.color.Color(line_color)
            back_color = renpy.color.Color(back_color)
            self.__line_color, self.__back_color = line_color, back_color

        @property
        def data(self):
            return self.__last_data

        def render(self, width, height, st, at):

            if (not self.__last_data) or (self.__st != st):
                self.__last_data = _bass_object.get_data(self._channel)
                self.__last_surf = _bass_object.draw_equalizer(
                    self.__last_data,
                    (self.width or width),
                    (self.height or height),
                    self.__line_color,
                    self.__back_color
                )
                self.__st = st

            render_object = renpy.Render(*self.__last_surf.get_size())
            render_object.blit(self.__last_surf, (0, 0))
            renpy.redraw(self, .0)
            return render_object

    class Pulsed(renpy.Displayable, NoRollback):

        __author__ = "Vladya"

        def __init__(
            self,
            child,
            default_zoom=1.5,
            attack_zoom=1.,
            channel="music"
        ):
            super(Pulsed, self).__init__()

            self.child = renpy.easy.displayable(child)
            if not isinstance(self.child, renpy.display.core.Displayable):
                raise TypeError(__("Тип 'child' должен быть displayable."))

            self._channel = renpy.audio.audio.get_channel(channel)

            self._default_zoom = float(default_zoom)
            self._attack_zoom = float(attack_zoom)

            self.__transform = None

        def event(self, ev, x, y, st):
            if self.__transform:
                return self.__transform.event(ev, x, y, st)

        def render(self, width, height, st, at):
            zoom = self._default_zoom + (
                (self._attack_zoom - self._default_zoom) * (
                    _bass_object.get_volume(self._channel)
                )
            )
            self.__transform = Transform(self.child, zoom=zoom)
            rend = renpy.render(self.__transform, width, height, st, at)
            render_object = renpy.Render(rend.width, rend.height)
            render_object.blit(rend, (0, 0))
            renpy.redraw(self, .0)
            return render_object
