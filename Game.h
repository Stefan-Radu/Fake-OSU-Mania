#ifndef GAME_H
#define GAME_H

#include <LedControl.h>

class Game {
public:

  /*
   * song 0 -> POC procedurally generated map. initially with no sound (ironical I know)
   * song 1 ... maybe I'll reach that point
   */

  Game(int driverCount = 1, int brightness = 2, int song = 0): lc(dinPin, clockPin, loadPin, driverCount) {
    for (int i = 0; i < driverCount; ++i) {
      lc.shutdown(0, false); // turn off power saving, enables display
      lc.setIntensity(0, brightness); // sets brightness (0~15 possible values)
      lc.clearDisplay(i); // clear screen    
    }
    startAnimation();

    lc.setRow(0, 3, B00011000);
    lc.setRow(0, 4, B00011000);
    lc.setRow(0, 5, B00011000);
  }

  void updateBrightness(int value) {
    if (0 <= value && value <= 15) {
      for (int i = 0; i < lc.getDeviceCount(); ++i) {
        lc.setIntensity(0, value); // sets brightness (0~15 possible values)    
      } 
    }
  }
  
private:

  LedControl lc;
  const int dinPin = 12, clockPin = 11, loadPin = 10;

  void startAnimation() {
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++ j) {
        lc.setLed(0, i, j, true);
        delay(10);
      }
    }
  
    for (int i = 0; i < 8; ++i) {
      for (int j = 0; j < 8; ++ j) {
        lc.setLed(0, i, j, false);
        delay(10);
      }
    }
  }
};

#endif
