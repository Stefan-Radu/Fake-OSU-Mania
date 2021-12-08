#ifndef GAME_H
#define GAME_H

#include <LedControl.h>
#include <LiquidCrystal.h>
#include "Joystick.h"
#include <time.h>

class Game {
public:

  #define MATRIX_HEIGHT 8
  #define MATRIX_WIDTH 8
  #define MAP_HEIGHT MATRIX_HEIGHT + 1
  /*
   * song 0 -> POC procedurally generated map. initially with no sound (ironical I know)
   * song 1 ... maybe I'll reach that point
   */

  Game(int driverCount = 1, int brightness = 2, int song = 0): lc(dinPin, clockPin, loadPin, driverCount) {
    srand(time(0));
    for (int i = 0; i < driverCount; ++i) {
      lc.shutdown(0, false); // turn off power saving, enables display
      lc.setIntensity(0, brightness); // sets brightness (0~15 possible values)
      lc.clearDisplay(i); // clear screen    
    }
    startAnimation();
    joystick = Joystick::getInstance();
  }

  void updateBrightness(int value) {
    if (0 <= value && value <= 15) {
      for (int i = 0; i < lc.getDeviceCount(); ++i) {
        lc.setIntensity(0, value); // sets brightness (0~15 possible values)    
      } 
    }
  }

  void updateStats(LiquidCrystal &lcd, int &s, int &l) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Score: ");
    lcd.print(s);
    lcd.setCursor(2, 1);
    lcd.print("Lives: ");
    lcd.print(l);
  }

  void playPOC(LiquidCrystal &lcd) {
    lastStateChange = 0;
    
    int score = 0, lives = 100, gameDelay = 150;
    bool joyStates[4] = {0, 0, 0, 0};
    updateStats(lcd, score, lives);
    float coefficient = 1, adder = 0.001;

    while (true) {
      unsigned long timeNow = millis();
      
      if (timeNow - lastStateChange > gameDelay) {
        bool change = false;
        int lastRow = currentRow - MATRIX_HEIGHT + 1;
        if (lastRow < 0) {
          lastRow += MAP_HEIGHT;
        }
        
        for (int j = 0; j < MATRIX_WIDTH; j += 2) {
          bool onMatrix = (matrixMap[lastRow] & (1 << j)) != 0;
          if (onMatrix != joyStates[3 - (j / 2)]) {
            lives -= 1;
            change = true;
          }
          else if (onMatrix == true) {
            score += 1;
            lives = min(lives + 1, 100);
            change = true;
          }
        }

        if (change) {
          updateStats(lcd, score, lives);
        }

        if (lives <= 0) {
          break;
        }

        currentRow += 1;
        if (currentRow == MAP_HEIGHT) {
          currentRow = 0;
        }
        generateNewLine(coefficient);
        displayMatrix();
        
        lastStateChange = timeNow;
        for (int i = 0; i < 4; ++i) {
          joyStates[i] = 0;   
        }
        coefficient += adder;
      }

      joystick->checkAllStates(joyStates);
    }
  }

  // ending let everything fall at 3 times the speed;
  
private:

  LedControl lc;
  const int dinPin = A2, clockPin = 11, loadPin = 10;
  Joystick* joystick = nullptr;

  byte matrixMap[MAP_HEIGHT] {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
  };
  int currentRow = MAP_HEIGHT - 2;
  unsigned long lastStateChange;

  void startAnimation() {
    for (int i = 0; i < MATRIX_HEIGHT; ++i) {
      for (int j = 0; j < MATRIX_WIDTH; ++ j) {
        lc.setLed(0, i, j, true);
        delay(10);
      }
    }
  
    for (int i = 0; i < MATRIX_HEIGHT; ++i) {
      for (int j = 0; j < MATRIX_WIDTH; ++ j) {
        lc.setLed(0, i, j, false);
        delay(10);
      }
    }
  }

  void displayMatrix() {
    int index = currentRow;
    for (int i = 0; i < MATRIX_HEIGHT; ++i) {
      lc.setRow(0, i, matrixMap[index]);
      --index;
      if (index == -1) {
        index = MAP_HEIGHT - 1;
      }
    }
  }

  void generateNewLine(float &coefficient) {
    int index = currentRow + 1;
    if (index == MAP_HEIGHT) {
      index = 0;
    }

    static const int threshold = 80;
    
    matrixMap[index] = B00000000;
    for (int i = 0; i < MATRIX_WIDTH; i += 2) {
      if (matrixMap[currentRow] & (3 << i) != 0) {
        if (rand() % 100 < min(threshold, 30 * coefficient)) {
          matrixMap[index] ^= (3 << i);
        }
      }
      else if (rand() % 100 < min(threshold, 5 * coefficient)) {
        matrixMap[index] ^= (3 << i);
      }
    };
  }
};

#endif
