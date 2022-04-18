//Example for JC Pro Macro board
//set up to work in Mac OS
//Includes support for 1306 display
//Reference: https://github.com/NicoHood/HID/blob/master/src/KeyboardLayouts/ImprovedKeylayouts.h
//Reference: https://arduinogetstarted.com/tutorials/arduino-button-long-press-short-press 
//
//To do:
//Figure out what to do with bottom support - remake offset so locks in?
//caps lock code? - does not work properly on Mac it seems

//========================================================

#include <SPI.h>
#include <Wire.h>

#include <SerialCommands.h>
char serial_command_buffer_[128];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r\n", " ");


// Declare variables etc=========================================

bool SW1 = 1; //encoder button
bool SW2 = 1; //lower-left keyswitch
bool SW3 = 1;
bool SW4 = 1; //lower-right keyswitch
bool SW5 = 1;
bool SW6 = 1; //upper-right keyswitch
bool SW7 = 1; //JCPM 2 key
bool SW8 = 1; //JCPM 2 key
bool SW9 = 1; //JCPM 2 key
bool SW10 = 1; //JCPM 2 mode switch

bool increment = 0;
bool decrement = 0;
long oldPosition;
long newPosition;
int inputMode = 0;
int LEDLight = 1;
int LEDCircle[4] = {0, 1, 3, 2};

//Long Press Setup==================================================

const int LONG_PRESS_TIME  = 500; // 1000 milliseconds
int lastState = 0;  // the previous state from the input pin
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

// Encoder setup =============================================

#include <Encoder.h>
#include <HID-Project.h>
Encoder myEnc(0,1);

// Screen setup =============================================

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
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
#define PIN        5 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 13 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel oober(21, 7, NEO_GRB + NEO_KHZ800);


#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels

//============================================================

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("ERROR: Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
}

void cmd_led(SerialCommands* sender) {
  char* lednum_str=sender->Next();
  
  if (lednum_str == NULL)
  {
    sender->GetSerial()->println("ERROR 1");
    return;
  }

  int lednum=atoi(lednum_str);

  char* r_str=sender->Next();
  if (r_str == NULL)
  {
    sender->GetSerial()->println("ERROR r");
    return;
  }

  int r=atoi(r_str);

    char* g_str=sender->Next();
  if (g_str == NULL)
  {
    sender->GetSerial()->println("ERROR g");
    return;
  }

  int g=atoi(g_str);

    char* b_str=sender->Next();
  if (b_str == NULL)
  {
    sender->GetSerial()->println("ERROR b");
    return;
  }

  int b=atoi(b_str);

  sender->GetSerial()->print(lednum);
  sender->GetSerial()->print(" ");
  sender->GetSerial()->print(r);
  sender->GetSerial()->print(" ");
  sender->GetSerial()->print(g);
  sender->GetSerial()->print(" ");
  sender->GetSerial()->println(b);


  pixels.setPixelColor(lednum, pixels.Color(r, g, b));
  
}

SerialCommand cmd_led_("LED", cmd_led);


void cmd_oober(SerialCommands* sender) {
  char* lednum_str=sender->Next();
  
  if (lednum_str == NULL)
  {
    sender->GetSerial()->println("ERROR 1");
    return;
  }

  int lednum=atoi(lednum_str);

  char* r_str=sender->Next();
  if (r_str == NULL)
  {
    sender->GetSerial()->println("ERROR r");
    return;
  }

  int r=atoi(r_str);

    char* g_str=sender->Next();
  if (g_str == NULL)
  {
    sender->GetSerial()->println("ERROR g");
    return;
  }

  int g=atoi(g_str);

    char* b_str=sender->Next();
  if (b_str == NULL)
  {
    sender->GetSerial()->println("ERROR b");
    return;
  }

  int b=atoi(b_str);

  sender->GetSerial()->print(lednum);
  sender->GetSerial()->print(" ");
  sender->GetSerial()->print(r);
  sender->GetSerial()->print(" ");
  sender->GetSerial()->print(g);
  sender->GetSerial()->print(" ");
  sender->GetSerial()->println(b);


  oober.setPixelColor(lednum, pixels.Color(r, g, b));
  
}

SerialCommand cmd_oober_("OOBER", cmd_oober);


void setup() {
  Serial.begin(57600);
  serial_commands_.AddCommand(&cmd_led_);  
 serial_commands_.AddCommand(&cmd_oober_);  
  serial_commands_.SetDefaultHandler(&cmd_unrecognized);
  
  pinMode(4, INPUT_PULLUP); //SW1 pushbutton (encoder button)
  pinMode(15, INPUT_PULLUP); //SW2 pushbutton
  pinMode(A0, INPUT_PULLUP); //SW3 pushbutton
  pinMode(A1, INPUT_PULLUP); //SW4 pushbutton
  pinMode(A2, INPUT_PULLUP); //SW5 pushbutton
  pinMode(A3, INPUT_PULLUP); //SW6 pushbutton


 pinMode(14, INPUT_PULLUP); //SW7 pushbutton
  pinMode(16, INPUT_PULLUP); //SW8 pushbutton
  pinMode(10, INPUT_PULLUP); //SW9 pushbutton
  pinMode(9, INPUT_PULLUP); //SW10 pushbutton - acts as mode switch

  randomSeed(analogRead(A9));
     
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setRotation(2); //sets rotation 1 through 4 (2 = 180º rotation vs 4)
  display.clearDisplay();
  // Draw text
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(30,0);             // Start at top-left corner
  display.print("Hello, world!");
  display.setCursor(30,30);  
  display.print("test");
  display.display();
  delay(10);
  display.setTextSize(3); 

//setup keyboard and mouse input
//perhaps add in a delay so that you can program before this starts up
Mouse.begin();
Keyboard.begin();
//BootKeyboard.begin(); - BootKeyboard use appears to give problems w/ Macintosh


//NeoPixel setup=========================================

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();


//  for(int i=0; i<NUMPIXELS; i++){
//    pixels.setPixelColor(i, pixels.Color(40, 40, 40));
//  }
  pixels.show(); // Show results

  oober.begin(); oober.clear();
  for(int i=0; i<21; i++){
    oober.setPixelColor(i, pixels.Color(40, 40, 40));
   }

  oober.show();
}


void loop() {

  SW1 = digitalRead(4);
  SW2 = digitalRead(15);
  SW3 = digitalRead(A0);
  SW4 = digitalRead(A1);
  SW5 = digitalRead(A2);
  SW6 = digitalRead(A3);
  SW7 = digitalRead(14);
  SW8 = digitalRead(16);
  SW9 = digitalRead(10);
  SW10 = digitalRead(9);

    serial_commands_.ReadSerial();
      pixels.show(); // Show results
      oober.show();

  newPosition = myEnc.read();
  
  if (newPosition > (oldPosition + 2)) { 
    increment = 1;
    delay(5);
    oldPosition = myEnc.read();
  }
  if (newPosition < (oldPosition - 2)) {
    decrement = 1;
    delay(5);
    oldPosition = myEnc.read();
  }

  //oldPosition = newPosition;
    
  //delay(100); **need a better way to debounce, or to send a single keystroke - maybe sense
  //when it's "off" as well and wait for next cycle
  //have some sort of secondary oldPosition comparison?

//================================

//screen();

//======select input mode:=======

if (inputMode == 0) volume();
if (inputMode == 1) volume();


//Serial.println(inputMode);

}


void screen(){
  display.clearDisplay();
  display.invertDisplay(0);
  display.setCursor(0,10);
  display.print(increment);
  display.print(decrement);
  display.print(" ");
  display.print(newPosition);
  display.println(LEDLight);
  display.print(SW1);
  display.print(SW2);
  display.print(SW3);
  display.print(SW4);
  display.print(SW5);
  display.print(SW6);
  display.print(inputMode);
  display.display();
  //Serial.println(SW1);
  //delay(10);
}

void volume(){

//====Pixels indicate input mode==============

//  pixels.clear();
//  for(int i=0; i<NUMPIXELS; i++){
//    pixels.setPixelColor(i, pixels.Color(10, 0, 0));
//  }
//  pixels.show(); // Show results

//===============================================

  if (increment == 1) {
            Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F12);
        Keyboard.releaseAll();
        delay(5);
       increment = 0;
        decrement = 0;
      }
      
  if (decrement == 1) {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F11);
        Keyboard.releaseAll();
        delay(5);
               increment = 0;
        decrement = 0;
      }

        if (SW10 == 0){ //tab to next browser tab Firefox or Chrome
         Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_END);
        Keyboard.releaseAll();
        delay(150);
        }
  if (SW9 == 0){ //tab to next browser tab Firefox or Chrome
         Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_HOME);
        Keyboard.releaseAll();
        delay(150);

      }
        if (SW8 == 0){ //tab to next browser tab Firefox or Chrome
         Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_PAGE_UP);
        Keyboard.releaseAll();
        delay(150);

      }
        if (SW7 == 0){ //tab to next browser tab Firefox or Chrome
         Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_PAGE_DOWN);
        Keyboard.releaseAll();
        delay(150);

      }

  if (SW6 == 0){ //tab to next browser tab Firefox or Chrome
         Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F10);
        Keyboard.releaseAll();
        delay(150);

      }
  if (SW5 == 0){ //tab to previous browser tab Firefox or Chrome
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F9);
        Keyboard.releaseAll();
        delay(150);
   
      }
  if (SW4 == 0) {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F8);
        Keyboard.releaseAll();
        delay(150);
      }
  if (SW3 == 0) {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F7);
        Keyboard.releaseAll();
        delay(150);
      }
  if (SW2 == 0) {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F6);
        Keyboard.releaseAll();
        delay(150);
      }
  if (SW1 == 0){ 
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(KEY_F5);
        Keyboard.releaseAll();
        delay(250);
  }
}

void jiggler(){
  return;

  Serial.print("commence to jiggling");
      //Consumer.write(MEDIA_VOLUME_UP);
      //Consumer.write(MEDIA_VOLUME_DOWN);
      long randNumber = random(-50, 50);
      long randNumber1 = random(-50, 50);
      long randNumber2 = random(-50, 50);
      Mouse.move(randNumber, randNumber1);
      delay(100);
      int xMap = map(randNumber, -50, 50, 0, 100);
      int yMap = map(randNumber1, -50, 50, 0, 100);
      int zMap = map(randNumber2, -50, 50, 0, 100);
      pixels.setPixelColor(0, pixels.Color(xMap, yMap, zMap));
      pixels.setPixelColor(2, pixels.Color(zMap,xMap,yMap));
      pixels.setPixelColor(1, pixels.Color(yMap, zMap, xMap));
      pixels.setPixelColor(3, pixels.Color(xMap, zMap, yMap));           
      pixels.show(); // Show results
      if (SW1 == 0){ 
      pixels.clear();
      for(int i=0; i<NUMPIXELS; i++){
      pixels.setPixelColor(i, pixels.Color(10, 0, 0));
    }
      pixels.show(); // Show results
      inputMode = 0;
      delay(200);
    }
}

//void FCPX() mode
