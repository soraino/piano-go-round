#include "wavTrigger.h"

#define KEYNUMS 14

wavTrigger wTrig;
unsigned long keyPrevMillis = 0;
const unsigned long keySampleIntervalMs = 25;
long longKeyPressCountMax = 80;    // 100 * 20 = 2000 ms
long longKeyPressCounts [KEYNUMS];

int prevKeyStates[KEYNUMS];

void setup() {
  for (int i = 0 ; i < KEYNUMS; i++) {
    longKeyPressCounts[i] = 0;
    prevKeyStates[i] = HIGH;
    pinMode(i + 2, INPUT_PULLUP);
  }
  // put your setup code here, to run once:

  // Start the wav trigger
  wTrig.start();
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
  Serial.begin(9600);
}

void keyPress(int i) {
  Serial.println(i);
  wTrig.trackPlayPoly(i + 1);
}

void loop() {
  // put your main code here, to run repeatedly:
  wTrig.masterGain(-40);
  if (millis() - keyPrevMillis >= keySampleIntervalMs) {
    keyPrevMillis = millis();
    int buttonStates [] = {digitalRead(0), digitalRead(1), digitalRead(2), digitalRead(3), digitalRead(4), digitalRead(5),
                           digitalRead(6), digitalRead(7), digitalRead(8), digitalRead(9), digitalRead(10), digitalRead(11),
                           digitalRead(12), digitalRead(13)
                          };
    for (int i = 0; i < KEYNUMS; i++) {
      if (prevKeyStates[i] == LOW && buttonStates[i] == HIGH) {
        longKeyPressCounts[i] = 0;
        Serial.print("released ");
        Serial.println(i);
      }
      else if (prevKeyStates[i] == HIGH && buttonStates[i] == LOW) {
        keyPress(i);
      }
      else if (buttonStates[i] == LOW && prevKeyStates[i] == LOW) {
        longKeyPressCounts[i]++;
      }
      prevKeyStates[i] = buttonStates[i];
    }
  }
}
