# dependencies found here: https://github.com/adafruit/Adafruit_CircuitPython_Bundle/releases/tag/20220928 7.x bundle
# adafruit_display_text adafruit_displayio_ssd1306
# keycodes available: https://github.com/adafruit/Adafruit_CircuitPython_HID/tree/main/adafruit_hid

import time
import digitalio
import board
import usb_hid

from adafruit_debouncer import Debouncer

from adafruit_hid.mouse import Mouse
from adafruit_hid.keyboard import Keyboard
from adafruit_hid.keycode import Keycode
from adafruit_hid.keyboard_layout_us import KeyboardLayoutUS
from adafruit_hid.consumer_control import ConsumerControl
from adafruit_hid.consumer_control_code import ConsumerControlCode
import random

# LED setup########################

import neopixel

LEDCirclePosition = 0
LEDCircle = [2, 3, 4, 5, 6, 7]
pixel_pin = board.D5
pixel_num = 12
pixels = neopixel.NeoPixel(pixel_pin, pixel_num, brightness=0.2)
boardpix = neopixel.NeoPixel(board.NEOPIXEL, 1, brightness=0.0) # disable onboard neopixel


import displayio
import terminalio
from adafruit_display_text import label
import adafruit_displayio_ssd1306

displayio.release_displays()

import busio

i2c = busio.I2C(board.D3, board.D2)

display_bus = displayio.I2CDisplay(i2c, device_address=0x3C)


WIDTH = 128
HEIGHT = 64  # Change to 64 if needed
BORDER = 5  # is this needed??

display = adafruit_displayio_ssd1306.SSD1306(
    display_bus, width=WIDTH, height=HEIGHT, rotation=180
)

# Make the display context
splash = displayio.Group()
display.show(splash)

color_bitmap = displayio.Bitmap(WIDTH, HEIGHT, 1)
color_palette = displayio.Palette(1)
color_palette[0] = 0xFFFFFF  # White

# Draw a label
text = "ENCOD:    TAB+ XXXX"
text_area = label.Label(terminalio.FONT, text=text, color=0xFFFFFF, x=5, y=5)
splash.append(text_area)
text = "VOL- VOL+ TAB- FNX"
text_area = label.Label(terminalio.FONT, text=text, color=0xFFFFFF, x=5, y=20)
splash.append(text_area)
text = "BACK STOP FORW FNX"
text_area = label.Label(terminalio.FONT, text=text, color=0xFFFFFF, x=5, y=35)
splash.append(text_area)

# rotary setup
# SPDX-FileCopyrightText: 2018 Kattni Rembor for Adafruit Industries
#
# SPDX-License-Identifier: MIT

import rotaryio
import board

encoder = rotaryio.IncrementalEncoder(board.D0, board.D1)
last_position = encoder.position

########################

buttons = {
    "SW1": board.D4,
    "SW2": board.SCK,
    "SW3": board.A0,
    "SW4": board.A1,
    "SW5": board.A2,
    "SW6": board.A3,
    "SW7": board.MISO,
    "SW8": board.MOSI,
    "SW9": board.D10,
    "SW10": board.D8,
}

for button in buttons:
    pin = digitalio.DigitalInOut(buttons[button])
    pin.direction = digitalio.Direction.INPUT
    pin.pull = digitalio.Pull.UP
    buttons[button] = Debouncer(pin)

keyboard = Keyboard(usb_hid.devices)
cc = ConsumerControl(usb_hid.devices)
mouse = Mouse(usb_hid.devices)

randomMode = 0

##Keys Function###########################


def keys():
    for button in buttons:
        buttons[button].update()

    if buttons["SW1"].fell:
        cc.send(ConsumerControlCode.MUTE)

    if buttons["SW2"].fell:
        keyboard.send(Keycode.LEFT_CONTROL, Keycode.ALT, Keycode.GUI, Keycode.F12)

    if buttons["SW3"].fell:
        keyboard.send(Keycode.LEFT_CONTROL, Keycode.ALT, Keycode.GUI, Keycode.F11)

    if buttons["SW10"].fell:
        global randomMode
        if randomMode:
            clearPixels()
        randomMode = not randomMode


def encoder1():
    global last_position
    global LEDCircle
    global LEDCirclePosition

    position = encoder.position
    delta = 0

    if last_position is None or position != last_position:
        # print('encoder postion', position)

        delta = position - last_position
        last_position = position

    if delta > 0:
        # print("encoder down - volume decrease")
        cc.send(ConsumerControlCode.VOLUME_DECREMENT)
        for LEDLoop in range(6):
            pixels[LEDCircle[LEDLoop]] = (0, 0, 0)  # Clear Key LEDs
        if LEDCirclePosition == 0:
            LEDCirclePosition = 5
        elif LEDCirclePosition > 0:
            LEDCirclePosition = LEDCirclePosition - 1

    if delta < 0:
        # print("endoder up - volume increase")
        cc.send(ConsumerControlCode.VOLUME_INCREMENT)
        for LEDLoop in range(6):
            pixels[LEDCircle[LEDLoop]] = (0, 0, 0)  # Clear Key LEDs
        if LEDCirclePosition == 5:
            LEDCirclePosition = 0
        elif LEDCirclePosition < 5:
            LEDCirclePosition = LEDCirclePosition + 1

    pixels[LEDCircle[LEDCirclePosition]] = (55, 0, 0)


# print('LEDCirclePosition', LEDCirclePosition)
# time.sleep(.2)


def randomMouseRoutine():
    randX = random.randint(-20, 20)
    randY = random.randint(-20, 20)
    for p in range(pixel_num):
        pixels[p] = (random.randint(-50,50),random.randint(-50,50),random.randint(-50,50))
    mouse.move(x=randX, y=randY)

def clearPixels():
    for p in range(pixel_num):
        pixels[p] = (0,0,0)

while True:

    keys()
    encoder1()
    if randomMode:
        randomMouseRoutine()
