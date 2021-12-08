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
    lcd.createChar(R_ARROW, rightArrow);
    lcd.createChar(U_ARROW, upArrow);
    lcd.createChar(D_ARROW, downArrow);
    lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lcd.noCursor();
    lcd.noBlink();

    // get settings and init
    loadFromStorage();
    game = new Game(1, settings.matrixBrightness);
    
    setContrast(); 
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

  struct {
    // values will be between 0 and 12 (aka count of slider blocks)
    int contrast = 6; // 0 - 12
    int matrixBrightness = 2; // 0 - 12
  } settings; // salvez si incarc chestii in si din eeprom
  
  const int RS = 13, enable = 6, d4 = A3, d5 = 4,
             d6 = 3, d7 = 7, v0 = 9;

  LiquidCrystal lcd;
  
  #define PLAY 3
  #define HIGHSCORE 0
  #define SETTINGS 1
  #define ABOUT 2

  #define ENTER_NAME 0
  #define CONTRAST 2
  #define MAT_BRIGHTNESS 3

  #define POC 1
  
  const String title = "Kinda OSU!";
  
  const int menuLengths[4] = {5, 5, 5, 5};
  const String menuSections[4][5] = { {
      " <" + title + ">",
      " Let's OSU!",
      " Highscore", // todo -> dupa POC ca sa am ce sa salvez
      " Settings",
      " About"
    }, {
      " <Settings>",
      " Enter name", // todo
      " Contrast",
      " Mat Brightnes",
      " Back"
    }, {
      " <About>",
      " Title: " + title,
      " By: Stefan R.",
      " Github: https://git.io/JDfIx",
      " Back"
    }, {
      " <Pick a Song>",
      " POC", // TODO
      " Song 1", // Ehey, macar sa ajung aici
      " Song 2",
      " Back",
    }
  };

  int highscores[3];

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
    lcd.print(" Press to save");
  
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
      
      int buttonState = joystick->getButton();
      if (buttonState == 1) {
        break;
      }
    }

    saveSettingsInStorage();
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
            // TODO highscore
            currentMenu = 0;
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
          game->playPOC(lcd);
        }
        if (sectionIndex == menuLengths[PLAY] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case SETTINGS:
        if (sectionIndex == CONTRAST) {
          sliderMenu(settings.contrast, &setContrast);
        } else if (sectionIndex == MAT_BRIGHTNESS) {
          sliderMenu(settings.matrixBrightness, &setMatrixBrightness);
        } else if (sectionIndex == menuLengths[SETTINGS] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case ABOUT:
        if (sectionIndex == menuLengths[ABOUT] - 1) {
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

  void setContrast() {
    int value = map(settings.contrast, 0, 12, 0, 255);
    analogWrite(v0, value);
  }

  void setMatrixBrightness() {
    int value = map(settings.matrixBrightness, 0, 12, 0, 15);
    game->updateBrightness(value);
  }

  void loadFromStorage() {
    EEPROM.get(0, settings);
    EEPROM.get(sizeof(settings), highscores);
  }

  void saveSettingsInStorage() {
    EEPROM.put(0, settings);
  }
};

#endif
