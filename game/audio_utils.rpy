
init python in _audio_utils:

    import audio_utils  # cython
    from store import NoRollback, HBox

    class Equalizer(renpy.Displayable, NoRollback):

        __author__ = "Vladya"

        def __init__(self, channel="music"):
            super(ShowValue, self).__init__()
            self._bass = audio_utils.BassDataHandler(
                renpy_audio_module=renpy.audio.audio
            )
            self._channel = renpy.audio.audio.get_channel(channel)

        def render(self, width, height, st, at):

            disp = HBox(
                *map(
                    lambda x: renpy.display.behavior.Bar(
                        value=x,
                        range=1.,
                        vertical=True,
                        xsize=15,
                        ysize=720
                    ),
                    self._bass.get_data(self._channel)
                ),
                spacing=0
            )
            rend = renpy.render(result, width, height, st, at)
            render_object = renpy.Render(rend.width, rend.height)
            render_object.blit(rend, (0, 0))
            renpy.redraw(self, .0)
            return render_object
