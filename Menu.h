#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>
#include "Joystick.h"
#include "Characters.h"
#include "Game.h"
#include <EEPROM.h>

// TODO: singleton
class Menu {
public:
  #define MAIN_MENU 0
  #define DISPLAY_WIDTH 16
  #define DISPLAY_HEIGHT 2

  Menu(): lcd(RS, enable, d4, d5, d6, d7) {
    joystick = Joystick::getInstance();
    sectionIndex = 1;
    currentMenu = MAIN_MENU;
    cursorRow = 1;
    
    // lcd related
    pinMode(v0, OUTPUT);
    lcd.createChar(BLOCK, block);
    lcd.createChar(L_ARROW, leftArrow);
    lcd.createChar(R_ARROW, rightArrow);
    lcd.createChar(U_ARROW, upArrow);
    lcd.createChar(D_ARROW, downArrow);
    lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lcd.noCursor();
    lcd.noBlink();

    // get settings and init
    loadFromStorage();
    updateHighscores(0);
    
    game = new Game(1, settings.matrixBrightness);
    
    setContrast();
    setDifficulty();
    // TODO un comment this
    showStartMessage();

    // update logic. this should be called sepparately
    showMenuSections();
  }

  void display() {
    bool updated = update();
    if (updated) {
      showMenuSections();
      scrollCount = 0;
      lastScrollTime = millis();
    } else {
      scrollLongLines();
    }
  }

private:

  int sectionIndex, currentMenu, cursorRow;

  Game *game;
  Joystick *joystick;

  #define PLAYER_NAME_LENGTH 13
  
  // settings are saved in and loaded from eeprom
  struct {
    int contrast = 6; // 0 - 12
    int matrixBrightness = 2; // 0 - 12
    int difficulty = 1; // 0 - 12
    char playerName[PLAYER_NAME_LENGTH];
  } settings;
  
  const int RS = 13, enable = 6, d4 = A3, d5 = 4,
             d6 = 5, d7 = 7, v0 = 9;

  LiquidCrystal lcd;
  
  #define PLAY 3
  #define HIGHSCORE 4
  #define SETTINGS 1
  #define ABOUT 2

  #define ENTER_NAME 0
  #define CONTRAST 2
  #define MAT_BRIGHTNESS 3
  #define DIFFICULTY 4

  #define POC 1
  
  const String title = "Kinda OSU!";
  
  const int menuLengths[5] = {5, 6, 5, 5, 5};
  String menuSections[5][6] = { {
      " <" + title + ">",
      " Let's OSU!",
      " Highscore",
      " Settings",
      " About"
    }, {
      " <Settings>",
      " Enter name", // todo
      " Contrast",
      " Mat Brightnes",
      " Difficulty",
      " Back"
    }, {
      " <About>",
      " Title: " + title,
      " By: Stefan R.",
      " Github: https://git.io/JDfIx",
      " Back"
    }, {
      " <Pick a Song>",
      " POC",
      " Song 1", // Ehey, macar sa ajung aici
      " Song 2",
      " Back",
    }, {
      " <Highscores>",
      " Player 1: ...",
      " Player 2: ...",
      " Player 3: ...",
      " Back",
    }
  };

  bool update() {
    bool c = updateCursor();
    bool m = switchMenu();
    if (m) {
      sectionIndex = 1;
      cursorRow = 1;
      // TODO somehow remember last index
    }
    return c || m;
  }

  bool updateCursor() {
    int dir = joystick->detectMoveY();
    if (dir == 0) return false;

    if (cursorRow == 0 && dir == 1 && sectionIndex < menuLengths[currentMenu] - 1) {
      cursorRow = 1;
    } else if (cursorRow == 1 && dir == -1 && sectionIndex > 1) {
      cursorRow = 0;
    } else if (dir == -1 && sectionIndex == 1) {
      cursorRow = 1;
    }

    sectionIndex += dir;
    if (sectionIndex == menuLengths[currentMenu]) {
      sectionIndex -= 1;
    } else if (sectionIndex == 0) {
      sectionIndex = 1;
    }
    return true;
  }

  void showStartMessage() {
    lcd.clear();
    lcd.setCursor(2, 0);
    lcd.print("Wellcome ");
    delay(500);
    lcd.print("and");
    lcd.setCursor(2, 1);
    delay(1000);
    lcd.print("Let's play");
    delay(1500);
    lcd.clear();
    delay(1000);
    lcd.setCursor(2, 0);
    lcd.print("OSU! Mania");
    delay(1500);
    lcd.setCursor(2, 1);
    lcd.print("Kinda :P");
    delay(2000);
    lcd.clear();
    delay(1000);
  }

  bool switchMenu() {
    int buttonState = joystick->getButton();
    if (buttonState != 1) {
      return false;
    }
    
    switch (currentMenu) {
      case MAIN_MENU:
        switch (sectionIndex) {
          case 1:
            currentMenu = PLAY;
            break;
          case 2:
            currentMenu = HIGHSCORE;
            break;
          case 3:
            currentMenu = SETTINGS;
            break;
          case 4:
            currentMenu = ABOUT;
            break;
        }
        break;
      case PLAY:
        if (sectionIndex == POC) {
          int score = game->playPOC(lcd);
          updateHighscores(score);
        } else if (sectionIndex == menuLengths[PLAY] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case SETTINGS:
        if (sectionIndex == ENTER_NAME) {
          selectNameMenu();
        } else if (sectionIndex == CONTRAST) {
          sliderMenu(settings.contrast, &setContrast);
        } else if (sectionIndex == MAT_BRIGHTNESS) {
          sliderMenu(settings.matrixBrightness, &setMatrixBrightness);
        } else if (sectionIndex == DIFFICULTY) {
          sliderMenu(settings.difficulty, &setDifficulty);
        } else if (sectionIndex == menuLengths[SETTINGS] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case ABOUT:
        if (sectionIndex == menuLengths[ABOUT] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case HIGHSCORE:
        if (sectionIndex == 4) {
          currentMenu = MAIN_MENU;
        }
        break;
    }
    return true;
  }

  void showMenuSections() {
    lcd.clear();
    int offset = 1 - cursorRow * 2; // mate
    lcd.setCursor(0, 1 - cursorRow);
    lcd.print(" " + menuSections[currentMenu][sectionIndex + offset]);
    lcd.setCursor(0, cursorRow);
    lcd.write(byte(R_ARROW));
    lcd.print(menuSections[currentMenu][sectionIndex]);

    if (sectionIndex < menuLengths[currentMenu] - 1) {
      lcd.setCursor(DISPLAY_WIDTH - 1, 1);
      lcd.write(byte(D_ARROW));
    }
    if (sectionIndex > 1 || (sectionIndex == 1 && cursorRow == 0)) {
      lcd.setCursor(DISPLAY_WIDTH - 1, 0);
      lcd.write(byte(U_ARROW));
    }
  }
  
  void sliderMenu(int &blockCount, void (Menu::*updateSettings)()) {
    static const int maxBlockCount = 12;
  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("- ");
    for (int i = 0; i < blockCount; ++i) {
      lcd.write(byte(BLOCK));
    }
    for (int i = blockCount; i < maxBlockCount; ++i) {
      lcd.print(" ");
    }
    lcd.print(" +");
    lcd.setCursor(0, 1);
    lcd.print(" Press to save!");
  
    while (true) {
      int dir = joystick->detectMoveX();
      if (dir == 1 && blockCount < maxBlockCount) {
        lcd.setCursor(2 + blockCount, 0);
        lcd.write(byte(BLOCK));
        blockCount += 1;
      } else if (dir == -1 && blockCount > 0) {
        lcd.setCursor(1 + blockCount, 0);
        lcd.print(" ");
        blockCount -= 1;
      }
  
      (this->*updateSettings)();
      
      if (joystick->getButton()) {
        break;
      }
    }
    saveSettingsInStorage();
  }

  void selectNameMenu() {
    lcd.setCursor(2, 0);
    lcd.print(settings.playerName);
    lcd.setCursor(2, 0);
    lcd.print("Press to save!");
    lcd.setCursor(2, 2);
    lcd.cursor();
    
    int letterIndex = 0;
    char name[13];
    strcpy(name, settings.playerName);
    
    while (!joystick->getButton()) {
      int x = joystick->detectMoveX();
      int y = joystick->detectMoveY();

      if (x != 0) {
        letterIndex += x;
        if (letterIndex < 0) {
          letterIndex = PLAYER_NAME_LENGTH - 2;
        } else if (letterIndex == PLAYER_NAME_LENGTH - 1) {
          letterIndex = 0;
        }
        lcd.setCursor(letterIndex + 2, 0);
      } else if (y != 0) {
        name[letterIndex] += y;
        switch (name[letterIndex]) {
          case 'a' - 1:
            name[letterIndex] = ' ';
            break;
          case 'Z' + 1:
            name[letterIndex] = ' ';
            break;
          case ' ' - 1:
            name[letterIndex] = 'Z';
            break;
          case ' ' + 1:
            name[letterIndex] = 'a';
            break;
          case 'z' + 1:
            name[letterIndex] = 'A';
            break;
          case 'A' - 1:
            name[letterIndex] = 'z';
            break;
        }
        lcd.print(name[letterIndex]);
        lcd.setCursor(letterIndex + 2, 0);
      }
    }
    saveSettingsInStorage();
  }

/*
 * 
 * ================= Scroll Logic =================
 * 
 */

  unsigned long lastScrollTime;
  const int scrollInterval = 500;
  int scrollCount;
  
  void scrollLongLines() {
      unsigned long timeNow = millis();
      if (scrollCount == 0 && timeNow - lastScrollTime < scrollInterval * 5) {
        return;
      } else if (scrollCount != 0 && timeNow - lastScrollTime < scrollInterval) {
        return;
      }

      int limit = menuSections[currentMenu][sectionIndex].length() - 14;
      if (scrollCount == limit && timeNow - lastScrollTime < scrollInterval * 5) {
        return;
      } else if (scrollCount == limit) {
        scrollCount = 0;
        lastScrollTime = millis();
        showMenuSections();
        return;
      }
     
      if (menuSections[currentMenu][sectionIndex].length() > 14) {
        lcd.scrollDisplayLeft();
        lcd.setCursor(DISPLAY_WIDTH - 1 + scrollCount, cursorRow);
        lcd.print(menuSections[currentMenu][sectionIndex][DISPLAY_WIDTH - 2 + scrollCount]);
        int offset = 1 - cursorRow * 2;
        lcd.setCursor(0, 1 - cursorRow);
        String padding = "  ";
        for (int i = 0; i < scrollCount; ++ i) {
          padding += " ";
        }
        lcd.setCursor(0, 1 - cursorRow);
        lcd.print(padding + menuSections[currentMenu][sectionIndex + offset]);
      }
        
      lastScrollTime = timeNow;
      scrollCount += 1;
  }

/*
 * 
 * ================= Highscore logic =================
 * 
 */
 
  #define HIGHSCORE_COUNT 3
  int highscores[HIGHSCORE_COUNT] = {0, 0, 0};
  char highscoreNames[HIGHSCORE_COUNT][PLAYER_NAME_LENGTH];

  void updateHighscores(int hs) {
    if (hs > highscores[0]) {
      // actual highscore numbers
      highscores[2] = highscores[1];
      highscores[1] = highscores[0];
      highscores[0] = hs;
      // names corresponding to the highscores
      strcpy(highscoreNames[2], highscoreNames[1]);
      strcpy(highscoreNames[1], highscoreNames[0]);
      strcpy(highscoreNames[0], settings.playerName);
    } else if (hs > highscores[1]) {
      highscores[2] = highscores[1];
      highscores[1] = hs;
      strcpy(highscoreNames[2], highscoreNames[1]);
      strcpy(highscoreNames[1], settings.playerName);
    } else if (hs > highscores[2]) {
      highscores[2] = hs;
      strcpy(highscoreNames[2], settings.playerName);
    }
    
    // update what is shown in the highscore menu
    for (int i = 0; i < 3; ++ i) {
      char *s = menuSections[HIGHSCORE][i + 1].c_str();
      sprintf(s, "%s: %d        ", highscoreNames[i], highscores[i]);
    }
    saveHighscoresInStorage();
  }

/*
 * 
 * ================= Settings and persistent storage =================
 * 
 */

  void setContrast() {
    int value = map(settings.contrast, 0, 12, 0, 255);
    analogWrite(v0, value);
  }

  void setMatrixBrightness() {
    int value = map(settings.matrixBrightness, 0, 12, 0, 15);
    game->updateBrightness(value);
  }

  void setDifficulty() {
    float value = 1.0 * map(settings.difficulty, 0, 12, 10000, 30000) / 10000;
    game->updateDifficulty(value);
  }

  void loadFromStorage() {
    unsigned int eeoffset = 0;
    EEPROM.get(eeoffset, settings);
    eeoffset += sizeof(settings);
    EEPROM.get(eeoffset, highscores);
    eeoffset += sizeof(highscores);
    EEPROM.get(eeoffset, highscoreNames);
  }

  void saveSettingsInStorage() {
    EEPROM.put(0, settings);
  }

  void saveHighscoresInStorage() {
    unsigned int eeoffset = sizeof(settings); 
    EEPROM.put(eeoffset, highscores);
    eeoffset += sizeof(highscores);
    EEPROM.put(eeoffset, highscoreNames);
  }
};

#endif
