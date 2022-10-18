import asynccp
import board
import rotaryio
import digitalio
import usb_hid
from adafruit_hid.consumer_control import ConsumerControl
from adafruit_hid.consumer_control_code import ConsumerControlCode
from adafruit_debouncer import Debouncer
from cpy_rotary import RotaryButton

cc = ConsumerControl(usb_hid.devices)

_do_nothing = tuple()


class JCPM:
    def __init__(self, encoder_button_action: list = _do_nothing):
        encoder_button = digitalio.DigitalInOut(board.D4)
        encoder_button.direction = digitalio.Direction.INPUT
        encoder_button.pull = digitalio.Pull.UP
        action = lambda: handle_encoder_press()
        self.encoder = RotaryButton(
            rotary=rotaryio.IncrementalEncoder(board.D0, board.D1),
            button=Debouncer(encoder_button),
            on_click=[action],
        )
        return self


def handle_encoder_press():
    cc.send(ConsumerControlCode.MUTE)


def run():
    encoder_press = lambda: handle_encoder_press()
    macroboard = JCPM(encoder_button_action=[encoder_press])
    asynccp.schedule(frequency=20, coroutine_function=macroboard.encoder.loop())

    asynccp.run()


if __name__ == "__main__":
    run()
