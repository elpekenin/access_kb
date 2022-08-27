from pekefw.audio._note import Note
from pekefw.audio._song import Song

# https://primesound.org/wp-content/uploads/2021/04/1.-Twinkle-Twinkle-Little-Star-1024x439.jpg
twinkle_star = Song(
    bpm=120,
    sounds=(
        (Note.C, 1), (Note.C, 1), (Note.G2, 1), (Note.G2, 1),
        (Note.A2, 1), (Note.A2, 1), (Note.G2, 2),
        (Note.F, 1), (Note.F, 1), (Note.E, 1), (Note.E, 1),
        (Note.D, 1), (Note.D, 1), (Note.C, 2),
        
        (Note.G2, 1), (Note.G2, 1), (Note.F, 1), (Note.F, 1),
        (Note.E, 1), (Note.E, 1), (Note.D, 2),
        (Note.G2, 1), (Note.G2, 1), (Note.F, 1), (Note.F, 1),
        (Note.E, 1), (Note.E, 1), (Note.D, 2),
        
        (Note.C, 1), (Note.C, 1), (Note.G2, 1), (Note.G2, 1),
        (Note.A2, 1), (Note.A2, 1), (Note.G2, 2),
        (Note.F, 1), (Note.F, 1), (Note.E, 1), (Note.E, 1),
        (Note.D, 1), (Note.D, 1), (Note.C, 2),
    ),
    stop=1/2
)

# https://static.tomplay.com/wp-content/uploads/2020/11/fox_Megalovania_from_undertale_flute_ea_fl_nemo_v1_19-10-2020_0001.png
megalovania = Song(
    bpm=270,
    sounds=(
        (Note.E, .5),   (Note.E, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
        (Note.D, .5),   (Note.D, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
         
        (Note.Cs, .5),  (Note.Cs, .5), (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
        (Note.C, .5),   (Note.C, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
         
        (Note.E, .5),   (Note.E, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
        (Note.D, .5),   (Note.D, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),

        (Note.Cs, .5),  (Note.Cs, .5), (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
        (Note.C, .5),   (Note.C, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),

        (Note.E, .5),   (Note.E, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
        (Note.D, .5),   (Note.D, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),

        (Note.Cs, .5),  (Note.Cs, .5), (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
        (Note.C, .5),   (Note.C, .5),  (Note.D2s, 1), (Note.B2, 1), (0, .2), (Note.A2s, 1), (Note.A2, 1), (Note.G2, 1), (Note.E, .5), (Note.G2, .5), (Note.A2, .5),
    ),
    stop=1/3
)
