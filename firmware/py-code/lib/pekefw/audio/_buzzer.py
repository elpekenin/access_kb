from machine import Pin, PWM
import time

            
class Buzzer:
    def __init__(self, *, pin, freq=40_000, duty=0.2): # hearing is [20Hz-20kHz], make it double bcs Nyquist
        """Freq in Hz"""
        self._pwm = PWM(Pin(pin))
        self._pwm.freq(freq)
        self._duty = duty
    
    def _play_sound(self, *, duration, freq, duty=None):
        """ Params:
                Duration (ms)
                Freq (Hz)
                Duty (0-1): low values = low valume, high values = trash
        """
        if duty is None:
            duty = self._duty
    
        if freq > 0: # accountfor silences
            self._pwm.freq(freq)
            self._pwm.duty_ns(int(duty/freq * 10**9))
        else:
            self._pwm.duty_ns(0)
        
        start = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), start) < duration:
            pass
        
        self._pwm.duty_ns(0)
        
            
    def play(self, song):
        for freq, duration in song._sounds:
            self._play_sound(duration=duration*song._duration, freq=freq)
            self._play_sound(duration=song._stop, freq=freq, duty=0)