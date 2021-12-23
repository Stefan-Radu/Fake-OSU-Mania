#ifndef GAME_H
#define GAME_H

#include <LedControl.h>
#include <LiquidCrystal_74HC595.h>
#include <Wire.h>
#include "Joystick.h"
#include "ButtonGroup.h"
#include "Globals.h"
#include "Master.h"


class Game {
public:

  #define MATRIX_HEIGHT 8
  #define MATRIX_WIDTH 8
  #define MAP_HEIGHT MATRIX_HEIGHT + 1

  Game(int brightness = 2): 
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
    master = Master::getInstance();
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

  int playSurvival() {
    unsigned long startTime = millis();
    for (int i = 0; i < MAP_HEIGHT; ++i) {
      matrixMap[i] = B00000000;
    }
    lc.clearDisplay(0);

    currentRow = 0, lastStateChange = 0, score = 0;
    int gameDelay = 250, scoreIncrement = difficulty;
    static const int maxLives = 50;
    float lives = maxLives, coeff = difficulty, 
          adder = 0.003, invCoeff = 3 - difficulty;

    updateInGameDisplayedStats(score, lives);
    
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
          updateInGameDisplayedStats(score, lives);
        }

        if (lives <= 0) {
          break;
        }

        currentRow += 1;
        if (currentRow == MAP_HEIGHT) {
          currentRow = 0;
        }
        displayMatrix();
        generateNewLine(coeff, sliderLength);
        
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

  int playSong(int song) {
    for (int i = 0; i < MAP_HEIGHT; ++i) {
      matrixMap[i] = B00000000;
    }
    lc.clearDisplay(0);

    master->selectSongTransmission(song);
    
    static const int maxLives = 30;
    currentRow = 0, lastStateChange = 0;
    score = 0, lives = maxLives;
    
    updateInGameDisplayedStats(score, lives);

    int sliderLength = 0, sliderColumn = 0, 
        melodyDisplayIndex = -1, melodyRefillIndex = 0,
        melodyNoteIndex = -1, lastLitColumnIndex = -1;

    bool canRefill = refillMelodyBuffer(melodyRefillIndex),
         finished = false;

    int badHits = 0, goodHits = 0;

    const int tempo = BASE_TEMPO + (difficulty - 2) * TEMPO_MULTIPLYER;
    const int wholeNoteDuration = 60000 / tempo * 4; // 60 s / tempo * 4 beats
    const int melodyBarDuration = wholeNoteDuration / WHOLE_NOTE_BAR_COUNT;

    // TODO replace with animation
    delay(2000);
         
    while (true) {
      unsigned long timeNow = millis(); 
      if (timeNow - lastStateChange > melodyBarDuration) {
        int lastRow = currentRow - MATRIX_HEIGHT + 1;
        if (lastRow < 0) {
          lastRow += MAP_HEIGHT;
        }

        bool anyLine = false;
        for (int j = 0; j < MATRIX_WIDTH; j += 2) {
          bool onMatrix = (matrixMap[lastRow] & (3 << j)) != 0;
          int columnIndex = 3 - (j / 2);
          // update sound to be played
          if (onMatrix == true) {
            anyLine = true;
            if (columnIndex != lastLitColumnIndex) {
              lastLitColumnIndex = columnIndex;
              melodyNoteIndex += 1;
              if (melodyNoteIndex == MELODY_BUFFER_LENGTH) {
                melodyNoteIndex = 0;
              }
              updateStats(badHits, goodHits);
              updateInGameDisplayedStats(score, lives);
              badHits = 0, goodHits = 0;
            }
          }
          // update stats & play sounds
          if (onMatrix != buttonStates[columnIndex]) {
            noTone(speakerPin);
            badHits += 1;
          } else if (onMatrix == true) {
            goodHits += 1;
            // for sound pauses
            if (melodyBuffer[melodyNoteIndex].note != 0) {
              tone(speakerPin, melodyBuffer[melodyNoteIndex].note);
            } else {
              noTone(speakerPin);
            }
          }
        }

        if (!anyLine && finished) {
          break;
        }

        if (lives <= 0) {
          break;
        }

        currentRow += 1;
        if (currentRow == MAP_HEIGHT) {
          currentRow = 0;
        }
        displayMatrix();
        
        if (!finished) {
          finished = generateNewLine(sliderLength, sliderColumn, 
              melodyDisplayIndex, melodyRefillIndex, canRefill);
        } else {
          getAndResetUpdateRow();
        }
        
        lastStateChange = timeNow;
      }

      buttonGroup->updateAllStates(buttonStates);
    }

    noTone(speakerPin);

    //endGameAnimation(gameDelay / 4, sliderLength);
    //displayEndGameStats(score, startTime);
    
    return score;
  }
  
private:

  LedControl lc;
  LiquidCrystal_74HC595 lcd;
  Master *master;
  
  Joystick* joystick = nullptr;
  ButtonGroup *buttonGroup = nullptr;
  
  float difficulty, score;

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

  int currentRow, lives;
  unsigned long lastStateChange;

  struct {
    int note, divider;
  } melodyBuffer[MELODY_BUFFER_LENGTH];
  
  bool buttonStates[ButtonGroup::buttonCount];
/*
 * 
 * ================= Melody Transfer =================
 * 
 */

  bool refillMelodyBuffer(int &melodyBufferIndex) {
    /*
     * Requests melody parts from slave.
     * returns true if there is more to receive
     * and false otherwise
     */
    Wire.requestFrom(SLAVE_NUMBER, MELODY_BYTES_TO_RECEIVE);
    while (true) {
      int value;
      Wire.readBytes((byte*)&value, sizeof(value));
      
      if (value == MELODY_SECTION_END) {
        return true;
      } else if (value == MELODY_END) {
        return false;
      }
      
      int note = value, divider;
      Wire.readBytes((byte*)&divider, sizeof(divider));

      melodyBuffer[melodyBufferIndex++] = {
        note, divider
      };
      if (melodyBufferIndex == MELODY_BUFFER_LENGTH) {
        melodyBufferIndex = 0;
      }
    }
  }

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

  void updateStats(float bad, float good) {
    static const int maxLives = 30;
    float perc = min(1, bad / (bad + good + 1));
    score += difficulty * (1 - perc);
    lives = min(maxLives, lives + sqrt((4 - difficulty)) * (2 - perc));
    lives = max(0, lives - difficulty * perc * 5);
  }
  
  /*
   * sets the row to be updated to 0
   * and returns its index
   */
  int getAndResetUpdateRow() {
    int updateRow = currentRow + 1;
    if (updateRow == MAP_HEIGHT) {
      updateRow = 0;
    }
    matrixMap[updateRow] = B00000000;
    return updateRow;
  }
  
  void generateNewLine(float coeff, int* sliderLength) {
    int updateRow = getAndResetUpdateRow();

    static const int threshold = 80;
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

  bool generateNewLine(int &sliderLength, int &sliderColumn, 
      int &melodyDisplayIndex, int &melodyRefillIndex, bool &canRefill) {
        
    if (sliderLength == 0) {
      melodyDisplayIndex += 1;
      if (melodyDisplayIndex == MELODY_BUFFER_LENGTH) {
        melodyDisplayIndex = 0;
      }
      if (melodyDisplayIndex == melodyRefillIndex) {
        if (canRefill) {
          canRefill = refillMelodyBuffer(melodyRefillIndex);
        } else {
          return true;
        }
      }
      if (melodyDisplayIndex != melodyRefillIndex) {
        sliderLength = WHOLE_NOTE_BAR_COUNT / 
            abs(melodyBuffer[melodyDisplayIndex].divider);
        if (melodyBuffer[melodyDisplayIndex].divider < 0) {
          sliderLength += sliderLength / 2;
        }
        int newColumn = melodyBuffer[melodyDisplayIndex].note % 4;
        if (newColumn == sliderColumn) {
          newColumn = (newColumn + 1) % 4;
        }
        sliderColumn = newColumn;
      } else {
        return true;
      }
    }
    
    --sliderLength;
    byte litBits = 0;
    int updateRow = getAndResetUpdateRow();
    if (melodyBuffer[melodyDisplayIndex].note == 0) {
      if (updateRow & 1) {
        litBits = 1;
      } else {
        litBits = 2;
      }
    } else {
      litBits = 3;
    } 
    matrixMap[updateRow] ^= (litBits << ((3 - sliderColumn) * 2));
    
    return false;
  }

/*
 * 
 * ================= Animations & Display =================
 * 
 */
 
  void updateInGameDisplayedStats(int s, int l) {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Score: ");
    lcd.print(s);
    lcd.setCursor(2, 1);
    lcd.print("Lives: ");
    lcd.print(l);
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
