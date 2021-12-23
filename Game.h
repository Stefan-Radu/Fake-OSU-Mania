#ifndef GAME_H
#define GAME_H

#include <LedControl.h>
#include <LiquidCrystal_74HC595.h>
#include "Joystick.h"
#include "ButtonGroup.h"
#include "Pins.h"


class Game {
public:

  #define MATRIX_HEIGHT 8
  #define MATRIX_WIDTH 8
  #define MAP_HEIGHT MATRIX_HEIGHT + 1

  Game(int brightness = 2, int song = 0): 
       lc(matrixDinPin, matrixClockPin, matrixLoadPin, 1),
       lcd(lcdDSPin, lcdClockPin, lcdLatchPin,
          lcdShiftRegisterRSPin, lcdShiftRegisterEPin,
          lcdShiftRegisterD4Pin, lcdShiftRegisterD5Pin,
          lcdShiftRegisterD6Pin, lcdShiftRegisterD7Pin) {
    
    randomSeed(analogRead(0));

    lc.shutdown(0, false); // turn off power saving, enables display
    lc.setIntensity(0, brightness); // sets brightness (0~15 possible values)
    lc.clearDisplay(0); // clear screen    
    
    startAnimation();
    joystick = Joystick::getInstance();
    buttonGroup = ButtonGroup::getInstance();
  }

  void updateBrightness(int value) {
    if (0 <= value && value <= 15) {
      for (int i = 0; i < lc.getDeviceCount(); ++i) {
        lc.setIntensity(0, value); // sets brightness (0~15 possible values)    
      }
    }
  }

  void updateDifficulty(float d) {
    difficulty = d;
  }

  int playPOC() {
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
    
    bool buttonStates[ButtonGroup::buttonCount] = {0, 0, 0, 0};
    updateInGameStats(score, lives);
    
    int sliderLength[MATRIX_WIDTH / 2] = {0, 0, 0, 0};

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
          if (onMatrix != buttonStates[3 - (j / 2)]) {
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
          updateInGameStats(score, lives);
        }

        if (lives <= 0) {
          break;
        }

        currentRow += 1;
        if (currentRow == MAP_HEIGHT) {
          currentRow = 0;
        }
        generateNewLine(coeff, sliderLength);
        displayMatrix();
        
        lastStateChange = timeNow;
        coeff += adder;
        invCoeff = max(0.0, invCoeff - adder);
      }

      buttonGroup->updateAllStates(buttonStates);
    }

    endGameAnimation(gameDelay / 4, sliderLength);
    displayEndGameStats(score, startTime);
    
    return score;
  }
  
private:

  LedControl lc;
  LiquidCrystal_74HC595 lcd;
  
  Joystick* joystick = nullptr;
  ButtonGroup *buttonGroup = nullptr;
  
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

/*
 * 
 * ================= Game Logic =================
 * 
 */

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
  
  void generateNewLine(float coeff, int* sliderLength) {
    int updateRow = currentRow + 1;
    if (updateRow == MAP_HEIGHT) {
      updateRow = 0;
    }

    static const int threshold = 80;
    
    matrixMap[updateRow] = B00000000;
    for (int i = 0; i < MATRIX_WIDTH; i += 2) {
      if (sliderLength[i / 2] != 0) {
        matrixMap[updateRow] ^= (3 << i);
        --sliderLength[i / 2];
        continue;
      }
      if (random(1000) < 2 * coeff) {
        sliderLength[i / 2] = 2 + random(6);
        continue; 
      }

      if (matrixMap[currentRow] & (3 << i) != 0) {
        continue;
      }
      
      if (random(100) < min(threshold, 7 * coeff)) {
        matrixMap[updateRow] ^= (3 << i);
      }
    };
  }

/*
 * 
 * ================= Animations & Display =================
 * 
 */

  void updateInGameStats(int &s, float &l) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Score: ");
    lcd.print(s);
    lcd.setCursor(2, 1);
    lcd.print("Lives: ");
    lcd.print(int(l));
  }

  void displayEndGameStats(int score, int startTime) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Score: ");
    lcd.print(score);
    lcd.setCursor(2, 1);
    lcd.print("Duration: ");
    lcd.print((millis() - startTime) / 1000);
    lcd.print("s");
    joystick->waitForPress();
  }

  void endGameAnimation(int d, int *sliderLength) {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print("Game Over!");
    lcd.setCursor(1, 1);
    lcd.print("You did GREAT!");
    
    for (int i = 0; i < MAP_HEIGHT * 20; ++i) {
      currentRow += 1;
      if (currentRow == MAP_HEIGHT) {
        currentRow = 0;
      }
      generateNewLine(8, sliderLength);
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
    joystick->waitForPress();
  }

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
};

#endif
