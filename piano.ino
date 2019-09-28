#include "wavTrigger.h"

wavTrigger wTrig;
unsigned long keyPrevMillis = 0;
const unsigned long keySampleIntervalMs = 25;
long longKeyPressCountMax = 80;    // 80 * 25 = 2000 ms
long longKeyPressCounts [] = {0,0,0};

byte prevKeyStates[] = {LOW,LOW,LOW};

void setup() {
  // put your setup code here, to run once:
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  // Start the wav trigger
  wTrig.start();
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  wTrig.stopAllTracks();
  wTrig.samplerateOffset(0);
  Serial.begin(9600);
}

void keyPress(int i){
  Serial.println(i);
  wTrig.trackPlayPoly(i+1);
}

void loop() {
  // put your main code here, to run repeatedly:
  wTrig.masterGain(1);
  if (millis() - keyPrevMillis >= keySampleIntervalMs) {
    keyPrevMillis = millis();
    byte buttonStates [3]= {digitalRead(2),digitalRead(3),digitalRead(4)};
    for(int i = 0; i < 3; i++){
      if (prevKeyStates[i] == LOW && buttonStates[i] == HIGH) {
        keyPress(i);
      }
      else if (prevKeyStates[i] == HIGH && buttonStates[i] == LOW) {
          //wTrig.trackPlayPoly(2);
      }
      else if (buttonStates[i] == HIGH) {
        longKeyPressCounts[i]++;
      }
      if(longKeyPressCountMax <= longKeyPressCounts[i]){
        longKeyPressCounts[i] = 0;
        prevKeyStates[i] = LOW;
      }else{
        prevKeyStates[i] = buttonStates[i];
      }
    }
  }
}
