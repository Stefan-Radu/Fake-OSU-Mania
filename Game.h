#ifndef GAME_H
#define GAME_H

#include <LedControl.h>
#include <LiquidCrystal_74HC595.h>
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

  Game(int driverCount = 1, int brightness = 2, int song = 0): 
       lc(dinPin, clockPin, loadPin, driverCount) {
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

  void updateStats(LiquidCrystal_74HC595 &lcd, int &s, float &l) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Score: ");
    lcd.print(s);
    lcd.setCursor(2, 1);
    lcd.print("Lives: ");
    lcd.print(int(l));
  }

  int playPOC(LiquidCrystal_74HC595 &lcd) {
    unsigned long startTime = millis();
    
    lastStateChange = 0;
    for (int i = 0; i < MAP_HEIGHT; ++i) {
      matrixMap[i] = B00000000;
    }
    lc.clearDisplay(0);
    
    int score = 0, gameDelay = 250, scoreIncrement = round(difficulty);
    static const int maxLives = 50;
    float lives = maxLives, coeff = difficulty, 
          adder = 0.003, invCoeff = 3 - difficulty;
    bool joyStates[4] = {0, 0, 0, 0};
    updateStats(lcd, score, lives);
    int updateOrder[4] = {0, 1, 2, 3};

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
            lives = max(0, lives - 1.0 * coeff);
            change = true;
          }
          else if (onMatrix == true) {
            score += scoreIncrement;
            lives = min(maxLives, lives + 2.0 * invCoeff);
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
        generateNewLine(coeff, updateOrder);
        displayMatrix();
        
        lastStateChange = timeNow;
        for (int i = 0; i < 4; ++i) {
          joyStates[i] = 0;   
        }
        coeff += adder;
        invCoeff = max(0.0, invCoeff - adder);
      }

      joystick->checkAllStates(joyStates);
    }

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Game Over!");
    lcd.setCursor(1, 1);
    lcd.print("You did GREAT!");
    
    endGameAnimation(gameDelay / 4, updateOrder);
    while (joystick->getButton() != 1);

    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Score: ");
    lcd.print(score);
    lcd.setCursor(2, 1);
    lcd.print("Duration: ");
    lcd.print((millis() - startTime) / 1000);
    lcd.print("s");
    while (joystick->getButton() != 1);
    
    return score;
  }

  void updateDifficulty(float d) {
    difficulty = d;
  }
  
private:

  LedControl lc;
  const int dinPin = 12, clockPin = 11, loadPin = 10;
  Joystick* joystick = nullptr;
  float difficulty;

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

  void endGameAnimation(int d, int *order) {
    for (int i = 0; i < MAP_HEIGHT * 20; ++i) {
      currentRow += 1;
      if (currentRow == MAP_HEIGHT) {
        currentRow = 0;
      }
      generateNewLine(8, order);
      displayMatrix();
      delay(d);
    }
    for (int i = 0; i < MAP_HEIGHT; ++i) {
      currentRow += 1;
      if (currentRow == MAP_HEIGHT) {
        currentRow = 0;
      }
      int updateRow = currentRow + 1;
      if (updateRow == MAP_HEIGHT) {
        updateRow = 0;
      }
      matrixMap[updateRow] = B00000000;
      displayMatrix();
      delay(d);
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

  void swap(int &x, int &y) {
    int k = x;
    x = y;
    y = k;
  }
  
  void generateNewLine(float coeff, int* order) {
    int updateRow = currentRow + 1;
    if (updateRow == MAP_HEIGHT) {
      updateRow = 0;
    }

    static const int threshold = 80;
    // random shuffle
    for (int i = 0; i < 4; ++ i) {
      swap(order[i], order[rand() % 4]);
    }
    
    matrixMap[updateRow] = B00000000;
    for (int i = 0; i < MATRIX_WIDTH; i += 2) {
      int index = order[i] * 2;
      if (matrixMap[currentRow] & (3 << index) != 0) {
        if (rand() % 100 < min(threshold, 50 * coeff)) {
          matrixMap[updateRow] ^= (3 << index);
          break;
        }
      } else if (rand() % 100 < min(threshold, 5 * coeff)) {
        matrixMap[updateRow] ^= (3 << index);
        break;
      }
    };
  }  
};

#endif
