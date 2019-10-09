#include <FastLED.h>

#define LED_TYPE NEOPIXEL
#define NUM_STRIPS 4
#define NUM_LEDS_PER_STRIP 60
#define NUM_LEDS_PER_STRIP_USED 60

unsigned long keyPrevMillis = 0;
const unsigned long keySampleIntervalMs = 25;

// CREATE THESE ARRAYS AND buttonStates[] ACCORDING TO NUM_STRIPS
byte prevKeyStates[NUM_STRIPS];
long longKeyPressCounts [NUM_STRIPS];
long longKeyPressCountMax = 80;    // 80 * 25 = 2000 ms

unsigned long letGoTime = 0;

// Set up memory block (an array) for storing and manipulating (to set/clear) LED data:
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

int hue = 0;
bool rainbowMessageShown = true;

void setup () {
  delay(2000); // power-up safety delay
  Serial.begin (115200);
  Serial.println("Rainbow diarrhoea initialised!");

  // Initialise these arrays with default values LOW:
  for ( int i=0; i < NUM_STRIPS; i++) {
    prevKeyStates[i] = HIGH;
    longKeyPressCounts[i] = 0;
  }
  
  pinMode (22, INPUT_PULLUP);
  pinMode (23, INPUT_PULLUP);
  pinMode (24, INPUT_PULLUP);
  pinMode (25, INPUT_PULLUP);
  
  // Tell FastLED there's 60 NEOPIXEL leds on OUT_PIN
  FastLED.addLeds<LED_TYPE, 44>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, 45>(leds[1], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, 46>(leds[2], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, 47>(leds[3], NUM_LEDS_PER_STRIP);

  // FastLED.setBrightness(255);
  FastLED.clear();
} // end of setup

void loop () {
  if ( millis() >= keyPrevMillis + keySampleIntervalMs) {
    keyPrevMillis = millis();
    byte buttonStates[NUM_STRIPS] = 
            { digitalRead(22), digitalRead(23), 
              digitalRead(24), digitalRead(25) };

    for(int i = 0; i < NUM_STRIPS; i++){
      if (prevKeyStates[i] == HIGH && buttonStates[i] == LOW) {
        // Key 'i' is pressed
        keyPress(i);
        FastLED.clear();
        // fill_solid( &(leds[i][0]), NUM_LEDS_PER_STRIP_USED, CHSV( i*20, 255, 224) );
      }
      else if (prevKeyStates[i] == LOW && buttonStates[i] == HIGH) {
        // Immediately when key 'i' is let go
        letGoTime = millis();
        keyRelease(i);
        rainbowMessageShown = false;
      } 
      else if (buttonStates[i] == LOW) {
        // Extended keypress of 'i' (previous high, current high)
        
        // PRESS BUTTON:
        // Each strip LED will be filled with a different starting hue
        // 256/7 = 36.57 , 256/12 = 21.33
        for (int j=0; j < NUM_LEDS_PER_STRIP_USED; j++) {
          // +hue for a different base hue every time
          
          //leds[i][j] = CHSV( i*36+(j/2), 255, 255);  // Don't really need fill_solid() if "number_of_leds == 1"
          hsv2rgb_spectrum(CHSV( (i)*36+(j/2), 255, 255), leds[i][j]);
          // fill_solid( &(leds[i][j]), 1, CHSV( i*36+(j/2), 255, 255) );  
          // fill_solid( &(leds[i][j]), 1, CRGB( 255, 255, 255) ); // All white for current testing
        }
        letGoTime = millis();
      } 
      else {
        // Fully let go 'i' (previous low, currently low)
        
        // FADING TO BLACK EFFECT: 
        // Reduce all brightnesses by 192/256 per loop() iteration.
        // Larger fraction = slower fade
        for(int j=0; j < NUM_LEDS_PER_STRIP_USED; j++) { leds[i][j].nscale8(192); }

        // SCREENSAVER: 
        // After 2s of no presses, hue++ to do a passive rainbow pattern:
        if ( millis() > letGoTime + 2000) {
          if (!rainbowMessageShown) {
            Serial.println("Rainbow diarrhoea reactivated.");
            rainbowMessageShown = true;
          }
          rainbowShift();
        } // end of if-2s
      } // end of buttonStates if-else
      
      FastLED.show();

     
      if (longKeyPressCounts[i] >= longKeyPressCountMax) {
        longKeyPressCounts[i] = 0;
        prevKeyStates[i] = HIGH;
        keyPress(i);
      } 
      else {
        prevKeyStates[i] = buttonStates[i];
      }
      
      
    } // end of for-loop for checking all buttons
  } // end of if-millis
} // end of loop()


// After 2s of no inputs received, all strips execute a shifting rainbow animation.
// hue/3 controls speed of change, j for strip index, k for a pixel index in a strip
void rainbowShift() {
  for (int j=0; j < NUM_STRIPS; j++) {
    for ( int k=0; k < NUM_LEDS_PER_STRIP_USED; k++){
      leds[j][k] = CHSV( hue/3+(15*j)+(k), 255, 80);
    }
  }
  // hue++ makes colours animate "downwards" to [0] pixel, while
  // hue-- makes colours animates "upwards" from [0] pixel.
  hue--;
}


// Serial monitor messages:

void keyPress(int buttonNum) {
  Serial.print(buttonNum);
  Serial.println(" is being pressed");
}

void keyRelease(int buttonNum) {
  Serial.print(buttonNum);
  Serial.println(" released");
}
