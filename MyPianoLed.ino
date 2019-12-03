#include <FastLED.h>
#include <AltSoftSerial.h>
#include <wavTrigger.h>

#define LED_TYPE NEOPIXEL
#define NUM_STRIPS 1
#define NUM_PIANO_KEYS 14
#define NUM_LEDS_PER_STRIP_USED 330

wavTrigger wTrig;
unsigned long keyPrevMillis = 0;
const unsigned long keySampleIntervalMs = 25;

// for analog pulsing LED
unsigned long ledUpdateMillis = 0;
int ledUpdateInterval = 20; // milliseconds
int ledFadeValue = 0; // current brightness of analog leds
byte ledDutyCycle = 223; // Max brightness of analog leds, range [0,255]
boolean isFadingUp = true;

// CREATE THESE ARRAYS AND buttonStates[] ACCORDING TO NUM_PIANO_KEYS
byte prevKeyStates[NUM_PIANO_KEYS];
long longKeyPressCounts[NUM_PIANO_KEYS];
long longKeyPressCountMax = 80; // 80 * 25 = 2000 ms
int pianoStartIndexes[NUM_PIANO_KEYS] = {0, 15, 33, 52, 73, 94, 118, 142, 166, 190, 217, 244, 271, 301};
int pianoEndIndexes[NUM_PIANO_KEYS] = {14, 32, 51, 72, 93, 117, 141, 165, 189, 218, 243, 270, 300, 330};

unsigned long letGoTime = 0;

// Set up memory block (an array) for storing and manipulating (to set/clear) LED data:
CRGB ledArray[NUM_LEDS_PER_STRIP_USED];
int hue = 0;
bool rainbowMessageShown = true;

void setup()
{
  delay(2000); // power-up safety delay
  Serial.begin(9600);
  Serial.println("Rainbow diarrhoea initialised!");

  // Initialise these arrays with default values LOW:
  for (int i = 0; i < NUM_PIANO_KEYS; i++)
  {
    prevKeyStates[i] = HIGH;
    longKeyPressCounts[i] = 0;
    pinMode(i, INPUT_PULLUP);
  }
  //for breathing/pulsing analog LED
  pinMode(44, OUTPUT);

  //  // Tell FastLED there's 60 NEOPIXEL leds on OUT_PIN
  FastLED.addLeds<LED_TYPE, 22>(ledArray, NUM_LEDS_PER_STRIP_USED);
  FastLED.clear();

  // Start the wav trigger
  wTrig.start();
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);

} // end of setup

void loop()
{
  wTrig.masterGain(1);
  // For breathing/pulsing analog LED without for loops
  if (millis() >ledUpdateMillis + ledUpdateInterval)
  {
    ledUpdateMillis = millis();
    if (isFadingUp) 
    {
      if (ledFadeValue <= ledDutyCycle) 
      {
        analogWrite(44, ledFadeValue);
        ledFadeValue = (ledFadeValue+1) * 1.1;
        if (ledFadeValue >= ledDutyCycle) 
        {
          ledFadeValue = ledDutyCycle;
          isFadingUp = false;
        }
      }
    }
    else // fading down to 0
    {
      if (ledFadeValue >= 0) 
      {
        analogWrite(44, ledFadeValue);
        ledFadeValue = (ledFadeValue/1.1) - 1;
        if (ledFadeValue <= 0) 
        {
          ledFadeValue = 0;
          isFadingUp = true;
        }
      }
    }
  } // end of pulsing analog LED

  if (millis() >= keyPrevMillis + keySampleIntervalMs)
  {
    keyPrevMillis = millis();
    int buttonStates[NUM_PIANO_KEYS] = {
        digitalRead(0), digitalRead(1), digitalRead(2), digitalRead(3), digitalRead(4),
        digitalRead(5), digitalRead(6), digitalRead(7), digitalRead(8), digitalRead(9),
        digitalRead(10), digitalRead(11), digitalRead(12), digitalRead(13)};

    for (int i = 0; i < NUM_PIANO_KEYS; i++)
    {
      if (prevKeyStates[i] == HIGH && buttonStates[i] == LOW)
      {
        // Key 'i' is pressed
        playPianoKey(i);
        keyPress(i);
        FastLED.clear();
        showKeyColour(i);
        // fill_solid( &(leds[i][0]), NUM_LEDS_PER_STRIP_USED, CHSV( i*20, 255, 224) );
      }
      else if (prevKeyStates[i] == LOW && buttonStates[i] == HIGH)
      {
        // Immediately when key 'i' is let go
        letGoTime = millis();
        keyRelease(i);
        rainbowMessageShown = false;
      }
      else if (buttonStates[i] == LOW)
      {
        letGoTime = millis();
      }
      else
      {
        fadeKeyColour(pianoStartIndexes[i], pianoEndIndexes[i]);
        // SCREENSAVER:
        // After 2s of no presses, hue++ to do a passive rainbow pattern:
        if (millis() > letGoTime + 2000)
        {
          if (!rainbowMessageShown)
          {
            Serial.println("Rainbow diarrhoea reactivated.");
            rainbowMessageShown = true;
          }
          rainbowShift();
        } // end of if-2s
      }   // end of buttonStates if-else

      FastLED.show();

      if (longKeyPressCounts[i] >= longKeyPressCountMax)
      {
        longKeyPressCounts[i] = 0;
        prevKeyStates[i] = HIGH;
        keyPress(i);
      }
      else
      {
        prevKeyStates[i] = buttonStates[i];
      }

    } // end of for-loop for checking all buttons
  }   // end of if-millis
} // end of loop()

// After 2s of no inputs received, all strips execute a shifting rainbow animation.
// hue/3 controls speed of change, j for strip index, k for a pixel index in a strip
void rainbowShift()
{
  for (int i = 0; i < NUM_LEDS_PER_STRIP_USED; i++)
  {
    
    ledArray[i] = CHSV(hue/3+i, 255, 80);
  }
  // hue++ makes colours animate "downwards" to [0] pixel, while
  // hue-- makes colours animates "upwards" from [0] pixel.
  hue--;
}

// Serial monitor messages:
void keyPress(int keyPressed)
{
  Serial.print(keyPressed);
  Serial.println(" is being pressed");
}
void keyRelease(int keyPressed)
{
  Serial.print(keyPressed);
  Serial.println(" released");
}

void playPianoKey(int keyPressed)
{
  // PRESS BUTTON:
  // Play the appropriate track based on key pressed
  wTrig.trackPlayPoly(keyPressed + 1);
}

void showKeyColour(int keyPressed)
{
  // PRESS BUTTON:
  // Each strip LED will be filled with a different starting hue
  int startIndex = pianoStartIndexes[keyPressed];
  int endIndex = pianoEndIndexes[keyPressed];
  for (int i = startIndex; i < endIndex; i++)
  {
    // +hue for a different base hue every time
    //leds[i][j] = CHSV( i*36+(j/2), 255, 255);  // Don't really need fill_solid() if "number_of_leds == 1"
    hsv2rgb_spectrum(CHSV((keyPressed)*36 + (i / 2), 255, 255), ledArray[i]);
    // fill_solid( &(leds[i][j]), 1, CHSV( i*36+(j/2), 255, 255) );
    // fill_solid( &(leds[i][j]), 1, CRGB( 255, 255, 255) ); // All white for current testing
  }
}

void showOctavesColour(int keyPressed)
{
  // PRESS BUTTON:
  // Each strip LED will be filled with a different starting hue
  int startIndex = pianoStartIndexes[keyPressed];
  int endIndex = pianoEndIndexes[keyPressed];
  for (int i = startIndex; i < endIndex; i++)
  {
    hsv2rgb_spectrum(CHSV((keyPressed)*36 + (i / 2), 255, 255), ledArray[i]);
  }
  if (keyPressed >= 6)
  {
    keyPressed - 6;
  }
  else
  {
    keyPressed + 6;
  }
  startIndex = pianoStartIndexes[keyPressed];
  endIndex = pianoEndIndexes[keyPressed];
  for (int i = startIndex; i < endIndex; i++)
  {
    hsv2rgb_spectrum(CHSV((keyPressed)*36 + (i / 2), 255, 255), ledArray[i]);
  }
}

void fadeKeyColour(int startIndex, int endIndex)
{
  // FADING TO BLACK EFFECT:
  // Reduce all brightnesses by 192/256 per loop() iteration.
  // Larger fraction = slower fade
  for (int i = startIndex; i < endIndex; i++)
  {
    ledArray[i].nscale8(192);
  }
}