// Example for JC Pro Macro board
// set up to work in Mac OS
// Includes support for 1306 display
// Reference: https://github.com/NicoHood/HID/blob/master/src/KeyboardLayouts/ImprovedKeylayouts.h
// Reference: https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press

// Declare variables etc=========================================

// DEBUG enables some serial debugging messages
// #define DEBUG
// This is the delay after each keypress
#define KEYDELAY 100
#define SSD1306_NO_SPLASH

#include <Arduino.h>
#include <HID-Project.h>

#include <ezButton.h>
ezButton SW1(4);
ezButton SW2(15);
ezButton SW3(A0);
ezButton SW4(A1);
ezButton SW5(A2);
ezButton SW6(A3);
ezButton SW7(14);
ezButton SW8(16);
ezButton SW9(10);
ezButton SW10(8);

bool underLight = false;
bool increment = false;
bool decrement = false;
int inputMode = 0;

int modeArray[] = {0, 1}; // adjust this array to modify sequence of modes - as written, change to {0, 1, 2, 3, 4, 5} to access all modes
int inputModeIndex = 0;
int modeArrayLength = (sizeof(modeArray) / sizeof(modeArray[0]));

#include <SerialCommands.h>
char serial_command_buffer_[128];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");

// Encoder setup =============================================
// This optional setting causes Encoder to use more optimized code
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
Encoder myEnc(1, 0); // if rotation is backwards, swap 0 and 1

// Screen setup =============================================

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NeoPixel setup =============================================

// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 5 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 15 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//============================================================

void cmd_unrecognized(SerialCommands *sender, const char *cmd)
{
    sender->GetSerial()->print("ERROR: Unrecognized command [");
    sender->GetSerial()->print(cmd);
    sender->GetSerial()->println("]");
}

void cmd_led(SerialCommands *sender)
{
    char *lednum_str = sender->Next();
    if (lednum_str == NULL)
    {
#ifdef DEBUG
        sender->GetSerial()->println("ERROR 1");
#endif
        return;
    }
    char lednum = atoi(lednum_str);

    char *r_str = sender->Next();
    if (r_str == NULL)
    {
#ifdef DEBUG
        sender->GetSerial()->println("ERROR r");
#endif
        return;
    }
    char r = atoi(r_str);

    char *g_str = sender->Next();
    if (g_str == NULL)
    {
#ifdef DEBUG
        sender->GetSerial()->println("ERROR g");
#endif
        return;
    }
    char g = atoi(g_str);

    char *b_str = sender->Next();
    if (b_str == NULL)
    {
#ifdef DEBUG
        sender->GetSerial()->println("ERROR b");
#endif
        return;
    }
    char b = atoi(b_str);

#ifdef DEBUG
    sender->GetSerial()->print(lednum);
    sender->GetSerial()->print(" ");
    sender->GetSerial()->print(r);
    sender->GetSerial()->print(" ");
    sender->GetSerial()->print(g);
    sender->GetSerial()->print(" ");
    sender->GetSerial()->println(b);
#endif

    pixels.setPixelColor(lednum, r, g, b);
    pixels.show();
}

SerialCommand cmd_led_("LED", cmd_led);

void setup()
{
    Serial.begin(57600);
    serial_commands_.AddCommand(&cmd_led_);
    serial_commands_.SetDefaultHandler(&cmd_unrecognized);
    // No TX and RX leds, please.
    pinMode(LED_BUILTIN_TX, INPUT);
    pinMode(LED_BUILTIN_RX, INPUT);

    randomSeed(analogRead(A9));

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        // Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.setRotation(2);              // sets rotation 1 through 4 (2 = 180º rotation vs 4)
    display.setTextColor(SSD1306_WHITE); // Draw white text

    // NeoPixel setup=========================================
    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    pixels.clear();
    pixels.show();
}

void loop()
{
    SW1.loop();
    SW2.loop();
    SW3.loop();
    SW4.loop();
    SW5.loop();
    SW6.loop();
    SW7.loop();
    SW8.loop();
    SW9.loop();
    SW10.loop();

    serial_commands_.ReadSerial();

    // Switch to program mode if left bottom and top right button are pressed at the same time
    if (SW2.isPressed() && SW7.isPressed())
    {
        screenBig("Upload Code!");
        delay(600000); // Wait for 10 minutes to reprogram, should be more than enough
    }

    static int oldPosition = 0;
    int newPosition = myEnc.read();
    if (newPosition > (oldPosition + 2))
    {
        increment = true;
    }
    else if (newPosition < (oldPosition - 2))
    {
        decrement = true;
    }
    oldPosition = newPosition;

    //=========change mode=================

    if (SW10.isPressed())
    {
        switch(inputModeIndex < modeArrayLength - 1) {
            case true:
                inputModeIndex++;
                break;
            default:
                inputModeIndex = 0;
                break;
        }
        inputMode = modeArray[inputModeIndex];
        pixels.clear();
        pixels.show();
        delay(KEYDELAY);
    }

    //======select input mode:=======

    switch (inputMode)
    {
    case 0:
        volume();
        break;
    case 1:
        jiggler();
        break;
    default:
        break;
    }
}

void volume()
{
    if (increment)
    {
        Consumer.write(MEDIA_VOLUME_UP);
    }
    else if (decrement)
    {
        Consumer.write(MEDIA_VOLUME_DOWN);
    }
    increment = decrement = false;

    if (SW1.isPressed())
    {
        Consumer.write(MEDIA_VOLUME_MUTE);
        delay(KEYDELAY);
    }

    if (SW6.isPressed())
    { // tab to next browser tab Firefox or Chrome
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
        delay(KEYDELAY);
    }
    if (SW5.isPressed())
    { // tab to previous browser tab Firefox or Chrome
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_TAB);
        Keyboard.releaseAll();
        delay(KEYDELAY);
    }
    if (SW3.isPressed())
    {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_WINDOWS);
        Keyboard.press(KEY_F11);
        Keyboard.releaseAll();
        delay(KEYDELAY);
    }
    if (SW2.isPressed())
    {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_WINDOWS);
        Keyboard.press(KEY_F12);
        Keyboard.releaseAll();
        delay(KEYDELAY);
    }

    if (SW8.isPressed())
    {
        uint8_t r = (!underLight) ? 140 : 0;
        uint8_t g = (!underLight) ? 0 : 0;
        uint8_t b = (!underLight) ? 130 : 0;
        for (int i = 8; i < 12; i++)
        {
            pixels.setPixelColor(i, r, g, b);
        }
        pixels.show();
        underLight = !underLight;
        delay(KEYDELAY);
    }

    screenVolume();
}

void jiggler()
{
    // Serial.print("commence to jiggling");
    long randNumber = random(-50, 50);
    long randNumber1 = random(-50, 50);
    Mouse.move(randNumber, randNumber1);
    delay(100);
    for (int i = 0; i <= 7; i++)
    {
        int xMap = map(random(-50, 50), -50, 50, 0, 100);
        int yMap = map(random(-50, 50), -50, 50, 0, 100);
        int zMap = map(random(-50, 50), -50, 50, 0, 100);
        pixels.setPixelColor(i, xMap, yMap, zMap);
    }
    pixels.show(); // Show results

    screenBig("Rando Mouse!");
}

//======================.96" oled screen=======================

void screenVolume()
{
    display.setTextSize(1);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MUTE |     |TAB+ |LIT");
    display.println("");
    display.println("VOL- |VOL+ |TAB- |   ");
    display.println("");
    display.println("MUTE |HAND |     |   ");
    display.display();
}

void screenBig(char *message)
{
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(0, 10);
    display.println(message);
    display.display();
}
