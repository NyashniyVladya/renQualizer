
label start:
    scene expression _renqualizer_test.Pulsed("background2.png"):
        align (.5, .5)
    show expression _renqualizer_test.Equalizer(1920, 540, "#00ff0080"):
        align (.5, 1.)
    play music "echo.mp3"
    "Some text."
    "Some text 2."
    return
