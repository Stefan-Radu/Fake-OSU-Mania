#include <Arduino.h>
#include "Menu.h"
#include <LedControl.h> //  need the library

const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); //DIN, CLK, LOAD, No. DRIVER

//void updateDisplay() {
//  for (int i = 0; i < 8; ++i) {
//    for (int j = 0; j < 8; ++ j) {
//      lc.setLed(0, i, j, true);
//      delay(20);
//    }
//  }
//
//  for (int i = 0; i < 8; ++i) {
//    for (int j = 0; j < 8; ++ j) {
//      lc.setLed(0, i, j, false);
//      delay(20);
//    }
//  }
//}

int main () {
  init ();  // initialize timers
  Serial.begin (9600);
  
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, 1); // sets brightness (0~15 possible values)
  lc.clearDisplay(0);// clear screen

  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++ j) {
      lc.setLed(0, i, j, true);
      delay(20);
    }
  }

  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++ j) {
      lc.setLed(0, i, j, false);
      delay(20);
    }
  }

  Menu menu;
  while (true) {
    menu.display();
  }
  
  Serial.flush (); // let serial printing finish
}
