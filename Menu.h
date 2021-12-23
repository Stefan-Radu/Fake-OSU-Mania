#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal_74HC595.h>
#include "Joystick.h"
#include "Characters.h"
#include "Game.h"
#include "Globals.h"
#include <EEPROM.h>

// TODO: singleton
class Menu {
public:
  #define MAIN_MENU 0
  #define DISPLAY_WIDTH 16
  #define DISPLAY_HEIGHT 2

  Menu(): lcd(lcdDSPin, lcdClockPin, lcdLatchPin,
          lcdShiftRegisterRSPin, lcdShiftRegisterEPin,
          lcdShiftRegisterD4Pin, lcdShiftRegisterD5Pin,
          lcdShiftRegisterD6Pin, lcdShiftRegisterD7Pin) {
    joystick = Joystick::getInstance();
    sectionIndex = 1;
    currentMenu = MAIN_MENU;
    cursorRow = 1;
    
    // lcd related
    pinMode(lcdV0Pin, OUTPUT);
    
    lcd.createChar(BLOCK, block);
    lcd.createChar(L_ARROW, leftArrow);
    lcd.createChar(R_ARROW, rightArrow);
    lcd.createChar(U_ARROW, upArrow);
    lcd.createChar(D_ARROW, downArrow);
    
    lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lcd.clear();
    lcd.noCursor();
    lcd.noBlink();
    
    // get settings and init    
    loadAllFromStorage();
    // used to update highscore menu
    updateHighscores(0);
    
    game = new Game(1, settings.matrixBrightness);
    
    setContrast();
    setDifficulty();
    // TODO un comment this
    //showStartMessage();

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

  #define PLAYER_NAME_LENGTH 6
  
  // settings are saved in and loaded from eeprom
  struct {
    int contrast = 6; // 0 - 12
    int matrixBrightness = 2; // 0 - 12
    int difficulty = 1; // 0 - 12
    char playerName[PLAYER_NAME_LENGTH + 1];
  } settings;
      
  LiquidCrystal_74HC595 lcd;
  
  #define HIGHSCORE 3
  #define SETTINGS 1
  #define ABOUT 2

  #define ENTER_NAME 1
  #define CONTRAST 2
  #define MAT_BRIGHTNESS 3
  #define DIFFICULTY 4
  
  #define MAX_SECTION_LINE_LENGTH 12
  
  const String title = "Kinda OSU!";
  
  const int menuLengths[5] = {5, 6, 5, 5};
  String menuSections[4][6] = { {
      "<" + title + ">",
      "Let's OSU!",
      "Highscore",
      "Settings",
      "About"
    }, {
      "<Settings>",
      "Enter name",
      "Contrast",
      "Mat Brightnes",
      "Difficulty",
      "Back"
    }, {
      "<About>",
      "Title: " + title,
      "By: Stefan R.",
      "Github: https://git.io/JDfIx",
      "Back"
    }, {
      "<Highscores>",
      "Player1: ___",
      "Player2: ___",
      "Player3: ___",
      "Back",
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
    int dir = -joystick->detectMoveY();
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
        if (sectionIndex == 1) {
          //int score = game->playPOC();
          //updateHighscores(score);
          game->playSong(0);
          currentMenu = MAIN_MENU;
        } else if (sectionIndex == 2) {
          currentMenu = HIGHSCORE;
        } else if (sectionIndex == 3) {
          currentMenu = SETTINGS;
        } else if (sectionIndex == 4) {
          currentMenu = ABOUT;
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

    Serial.println("menu");
    Serial.println(currentMenu);
    return true;
  }

  void showMenuSections() {
    lcd.clear();
    int offset = 1 - cursorRow * 2; // mate
    lcd.setCursor(0, 1 - cursorRow);
    lcd.print("  " + menuSections[currentMenu][sectionIndex + offset]);
    lcd.setCursor(0, cursorRow);
    lcd.write(byte(R_ARROW));
    lcd.print(" ");
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
    lcd.setCursor(1, 1);
    lcd.print("Press to save!");
  
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

    int offset = (DISPLAY_WIDTH - PLAYER_NAME_LENGTH) / 2;
    
    lcd.clear();
    lcd.setCursor(offset - 1, 0);
    lcd.print('<');
    lcd.print(settings.playerName);
    lcd.setCursor(DISPLAY_WIDTH - offset, 0);
    lcd.print('>');
    lcd.setCursor(1, 1);
    lcd.print("Press to save!");
    lcd.setCursor(offset, 0);
    lcd.cursor();
    
    int letterIndex = 0;
    char name[PLAYER_NAME_LENGTH + 1];
    strcpy(name, settings.playerName);

    for (int i = strlen(name); i < PLAYER_NAME_LENGTH; ++i) {
      name[i] = ' ';
    }
    
    while (!joystick->getButton()) {
      int x = joystick->detectMoveX();
      int y = joystick->detectMoveY();
      if (x != 0) {
        letterIndex += x;
        if (letterIndex < 0) {
          letterIndex = PLAYER_NAME_LENGTH - 1;
        } else if (letterIndex == PLAYER_NAME_LENGTH) {
          letterIndex = 0;
        }
        lcd.setCursor(letterIndex + offset, 0);
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
        lcd.setCursor(letterIndex + offset, 0);
      }
      delay(40);
    }

    for (int i = PLAYER_NAME_LENGTH; i > 0; --i) {
      if (name[i - 1] != ' ') {
        name[i] = '\0';
        break;
      }
    }
    
    strcpy(settings.playerName, name);
    saveSettingsInStorage();
    lcd.noCursor();
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
  char highscoreNames[HIGHSCORE_COUNT][PLAYER_NAME_LENGTH + 1];

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
      // copy at most <length> caracters.
      // basically pad with spaces at the end
      snprintf(s, MAX_SECTION_LINE_LENGTH + 1, "%s: %d           \0",
               highscoreNames[i], highscores[i]);
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
    analogWrite(lcdV0Pin, value);
  }

  void setMatrixBrightness() {
    int value = map(settings.matrixBrightness, 0, 12, 0, 15);
    game->updateBrightness(value);
  }

  void setDifficulty() {
    float value = 1.0 * map(settings.difficulty, 0, 12, 10000, 30000) / 10000;
    game->updateDifficulty(value);
  }

  void loadAllFromStorage() {
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
