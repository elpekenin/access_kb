import time
import _thread

from pekefw.rgb import WS2812
from pekefw.audio import Buzzer, songs

if __name__ == "__main__":
    stop = False
    def rgb():
        ws = WS2812(leds=16, pin=0)
        global stop
        while True:
            for color in WS2812.COLORS:
                if stop:
                    print("Quitting at core 1")    
                    ws.set_color_all(WS2812.BLACK)
                    return
                ws.anim_chase(color, 0.1)
                time.sleep(0.4)

            
    t1 = _thread.start_new_thread(rgb, ())

    
    #buzzer = Buzzer(pin=1, duty=0.1)
    #buzzer.play(songs.twinkle_star)
    
    buzzer = Buzzer(pin=1, duty=0.09)
    try:
        buzzer.play(songs.megalovania)
    except KeyboardInterrupt:
        stop = True
        print("Quitting")
        del buzzer