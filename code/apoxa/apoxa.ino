#include <IoAbstraction.h>
#include <TaskManagerIO.h>
#include <HID-Project.h>

#define DEBUG

// Set higher hold threshold, default is 20
#define HOLD_THRESHOLD 100

// These defines are arbitrary, but this is a simple hack to build a hash-like array.
#define SW1 0
#define SW2 1
#define SW3 2
#define SW4 3
#define SW5 4
#define SW6 5
#define SW7 6
#define SW8 7
#define SW9 8
#define SW10 9
#define ENCLEFT 10
#define ENCRIGHT 11

struct button
{
    unsigned char s[ENCRIGHT + 1];
    button()
    {
        s[SW1] = 4;      // encoder button
        s[SW2] = 15;     // bottom row, leftmost button
        s[SW3] = A0;     // bottom row, second-to-leftmost button
        s[SW4] = A1;     // bottom row, second-to-rightmost button
        s[SW9] = 10;     // bottom row, rightmost-button
        s[SW7] = 14;     // top row, right button
        s[SW10] = 8;     // left button under encoder, "switch mode" button
        s[ENCLEFT] = 1;  // encoder left spin
        s[ENCRIGHT] = 0; // encoder right spin
    }
};

button button;

// the maximum (0 based) value that we want the encoder to represent.
const short maximumEncoderValue = 32767;
short EncoderOldValue = maximumEncoderValue / 2;

auto boardIo = internalDigitalIo();
MultiIoAbstractionRef multiIo = multiIoExpander(100);

// Screen setup =============================================

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET 4        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

bool underLight = false;

//
// When the spinwheel is clicked, this function will be run as we registered it as a callback
//
void muteVolume(pinid_t pin, bool heldDown)
{
#ifdef DEBUG
    Serial.println("Encoder button pressed");
#endif
    Consumer.write(MEDIA_VOLUME_MUTE);
    delay(100);
}

//
// Each time the encoder value changes, this function runs, as we registered it as a callback
//
void changeVolume(short newValue)
{
    // Fix startup of pad
    if (newValue == EncoderOldValue)
        return;

#ifdef DEBUG
    Serial.print("Encoder change ");
    Serial.println(newValue);
#endif

    Consumer.write((newValue > EncoderOldValue) ? MEDIA_VOLUME_UP : MEDIA_VOLUME_DOWN);
    EncoderOldValue = newValue;
    delay(20);
}

void setup()
{
    delay(5000); // WAIT FOR PROGRAMMING!
    taskManager.scheduleOnce(1, &screenWelcome, TIME_SECONDS);
#ifdef DEBUG
    Serial.begin(115200);
#endif

    // No TX and RX leds, please.
    pinMode(LED_BUILTIN_TX, INPUT);
    pinMode(LED_BUILTIN_RX, INPUT);

    randomSeed(analogRead(A9));

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        for (;;)
            ; // Don't proceed, loop forever
    }
    display.setRotation(2);              // sets rotation 1 through 4 (2 = 180º rotation vs 4)
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.invertDisplay(0);

    // Initialise NeoPixels
    pixels.begin();
    pixels.clear();
    pixels.show();

    // First we set up the switches library, giving it the task manager and tell it to use arduino pins
    // We could also of chosen IO through an i2c device that supports interrupts.
    // If you want to use PULL DOWN instead of PULL UP logic, change the true to false below.
    switches.initialise(ioUsingArduino(), true);

    // Global listener for switch mode button
    // single presses cycle through modes
    // if button is held, code upload mode is entered
    switches.addSwitch(button.s[SW10], [](pinid_t pin, bool held)
                       {
#ifdef DEBUG
        Serial.println("SWITCH MODE");
#endif
        resetSwitches();
        if (held) {
          screenBig("Upload Code!");
          delay(3600000);
        } });

    // Setup default mode for buttons
    taskManager.scheduleOnce(3, &DefaultMode, TIME_SECONDS);
}

void loop()
{
    taskManager.runLoop();
}

void resetSwitches()
{
    for (char i = 0; i < (sizeof(button.s) / sizeof(button.s[0])); i++)
    {
        // skip "switch mode" button
        if (i == SW10)
            continue;
        switches.removeSwitch(button.s[i]);
    }
}

void DefaultMode()
{
    // now we add the switches, we dont want the spinwheel button to repeat, so leave off the last parameter
    // which is the repeat interval (millis / 20 basically) Repeat button does repeat as we can see.
    switches.addSwitch(button.s[SW1], &muteVolume);

    // now we set up the rotary encoder, first we give the A pin and the B pin.
    // we give the encoder a max value of 128, always minumum of 0.
    setupRotaryEncoderWithInterrupt(button.s[ENCLEFT], button.s[ENCRIGHT], &changeVolume);
    switches.changeEncoderPrecision(maximumEncoderValue, EncoderOldValue);

    switches.addSwitch(button.s[SW2], [](pinid_t pin, bool held)
                       {
        if (held) return;
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_WINDOWS);
        Keyboard.press(KEY_F12);
        Keyboard.releaseAll();
        delay(50); });
    switches.addSwitch(button.s[SW3], [](pinid_t pin, bool held)
                       {
        if (held) return;
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_WINDOWS);
        Keyboard.press(KEY_F11);
        Keyboard.releaseAll();
        delay(50); });

    switches.addSwitch(button.s[SW7], [](pinid_t pin, bool held)
                       {
        if (held) return;
        for (int i = 8; i < 12; i++)
        {
            if (underLight) {
                pixels.setPixelColor(i, 0,0,0);
            }
            else {
                pixels.setPixelColor(i, 30,255,70);
            }
        }
        underLight = !underLight;
        pixels.show(); // Show results
        delay(100); });

    screenVolume();
}

void screenClear()
{
    display.clearDisplay();
    display.display();
}

void screenWelcome()
{
    screenBig("Welcome");
}

void screenBig(char *message)
{
    display.clearDisplay();
    display.setTextSize(3);
    display.setCursor(0, 10);
    display.println(message);
    display.display();
}

void screenVolume()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("MUTE |     |     |LIT");
    display.println("");
    display.println("VOL- |VOL+ |     |   ");
    display.println("");
    display.println(" MIC |HAND |     |   ");
    display.println("");
    display.display();
}
