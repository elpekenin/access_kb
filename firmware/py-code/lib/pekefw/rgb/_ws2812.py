import array
from machine import Pin
import rp2
import time


@rp2.asm_pio(sideset_init=rp2.PIO.OUT_LOW, out_shiftdir=rp2.PIO.SHIFT_LEFT, autopull=True, pull_thresh=24)
def _ws2812():
    T1 = 2
    T2 = 5
    T3 = 3
    wrap_target()
    label("bitloop")
    out(x, 1)               .side(0)    [T3 - 1]
    jmp(not_x, "do_zero")   .side(1)    [T1 - 1]
    jmp("bitloop")          .side(1)    [T2 - 1]
    label("do_zero")
    nop()                   .side(0)    [T2 - 1]
    wrap()


class WS2812:
    BLACK  = (  0,   0,   0)
    RED    = (255,   0,   0)
    YELLOW = (255, 150,   0)
    GREEN  = (  0, 255,   0)
    CYAN   = (  0, 255, 255)
    BLUE   = (  0,   0, 255)
    PURPLE = (180,   0, 255)
    WHITE  = (255, 255, 255)
    COLORS = (BLACK, RED, YELLOW, GREEN, CYAN, BLUE, PURPLE, WHITE)
    
    def __init__(self, *, leds, pin, freq=8_000_000, sleep=10, brightness=0.01):
        self._leds  = leds
        self._sleep = sleep
        self.brightness = brightness # can change at runtime
        
        # Create the StateMachine with the ws2812 program, outputting on pin
        self._sm = rp2.StateMachine(0, _ws2812, freq=freq, sideset_base=Pin(pin))
        
        # Start the StateMachine, it will wait for data on its FIFO.
        self._sm.active(1)
        
        # Display a pattern on the LEDs via an array of LED RGB values.
        self._ar = array.array("I", [0 for _ in range(leds)])        

    @staticmethod
    def _encode(color):
        # Transform an RGB tuple into a 6byte number in GRB (used by most WS2812's)
        return (color[1]<<16) + (color[0]<<8) + color[2]
    
    def _show(self):
        # Temp_buffer to send data to LEDs
        dimmer_ar = array.array("I", [0 for _ in range(self._leds)])
        
        for i, c in enumerate(self._ar):
            # Un-code values and scale by brightness
            r = int(((c >>  8) & 0xFF) * self.brightness)
            g = int(((c >> 16) & 0xFF) * self.brightness)
            b = int(((c >>  0) & 0xFF) * self.brightness)
            
            # Re-code value
            dimmer_ar[i] = self._encode((r, g, b)) 
            #dimmer_ar[i] = (g<<16) + (r<<8) + b # bitwise operator would be faster
            
        self._sm.put(dimmer_ar, 8)
        time.sleep_ms(self._sleep)

    def set_color(self, i, color):
        if isinstance(color, tuple):    # if the input is a tuple, convert to encoded value
            color = self._encode(color) 
        self._ar[i] = color

    def set_color_all(self, color):
        _color = self._encode(color)
        for i in range(self._leds):
            self.set_color(i, _color)
        self._show()
            
    def anim_chase(self, color, step=0.3):
        for i in range(self._leds):
            self.set_color(i, color)
            time.sleep(step)
            self._show()
            
