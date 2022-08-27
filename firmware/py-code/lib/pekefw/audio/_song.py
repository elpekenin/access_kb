from micropython import const

class Song:
    def __init__(self, *, bpm, sounds: [Note, float], stop=1/4):
        """The sounds are defined by a note and its duration, as a factor, 1 being a crotchet, 0.5 a quaver and so on"""
        self._duration = 60_000/bpm # time (ms) representing each time "block"
        self._sounds   = sounds
        self._stop     = self._duration*stop
        