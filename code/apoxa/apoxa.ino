#include <Eventually.h>

/*
 * This just shows a simple way of using the library.
 * Pushing a button moves the Arduino back-and-forth
 * from a fast blink to a slow blink.
 */

#define LIGHT_PIN 5
#define BUTTON_PIN 4

bool speed = LOW;
EvtManager mgr;
bool pin_state = LOW;

bool blink()
{
    pin_state = !pin_state;
    Serial.println(pin_state);
    return false;
}

bool set_speed()
{
    mgr.resetContext();
    mgr.addListener(new EvtPinListener(BUTTON_PIN, 40, (bool)LOW, (EvtAction)set_speed));
    speed = !speed; // Change speeds
    if (speed == HIGH)
    {
        mgr.addListener(new EvtTimeListener(250, true, (EvtAction)blink));
    }
    else
    {
        mgr.addListener(new EvtTimeListener(1000, true, (EvtAction)blink));
    }

    return true;
}

void setup()
{
    delay(5000); // delay to allow programming DO NOT REMOVE!!!!!
    Serial.begin(115200);

    pinMode(LIGHT_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    set_speed();
}

USE_EVENTUALLY_LOOP(mgr)
