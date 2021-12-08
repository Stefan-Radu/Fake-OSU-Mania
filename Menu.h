#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>
#include "Joystick.h"
#include "Characters.h"

// TODO: singleton
class Menu {
public:
  #define MAIN_MENU 0

  Menu(): lcd(RS, enable, d4, d5, d6, d7), joystick() {
    sectionIndex = 1;
    currentMenu = MAIN_MENU;
    cursorRow = 1;
    pinMode(v0, OUTPUT);
    lcd.createChar(BLOCK, block);
    lcd.createChar(R_ARROW, rightArrow);
    lcd.createChar(U_ARROW, upArrow);
    lcd.createChar(D_ARROW, downArrow);
    lcd.begin(displayWidth, displayHeight);
    lcd.noCursor();
    lcd.noBlink();
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
  const int displayWidth = 16, displayHeight = 2;

  Joystick joystick;

  struct {
    int contrast = 120;
    int brightness = 2; // 0 - 15
  } settings; // salvez si incarc chestii in si din eeprom
  
  const int RS = 13, enable = 6, d4 = 5, d5 = 4,
             d6 = 3, d7 = 7, v0 = 9;

  LiquidCrystal lcd;
  
  #define PLAY 3
  #define HIGHSCORE 0
  #define SETTINGS 1
  #define ABOUT 2

  #define ENTER_NAME 0
  #define CONTRAST 2
  #define BRIGHTNESS 3

  #define POC 1
  
  const String title = "Kinda OSU!";
  
  const int menuLengths[4] = {5, 5, 5, 5};
  const String menuSections[4][5] = { {
      " <" + title + ">",
      " Let's OSU!", // -> song list + procedural care o sa fie POC
      " Highscore", // todo -> dupa POC ca sa am ce sa salvez
      " Settings",
      " About"
    }, {
      " <Settings>",
      " Enter name", // todo
      " Contrast",
      " Brightnes", // TODO
      " Back"
    }, {
      " <About>",
      " Title: " + title,
      " By: Stefan R.",
      " Github: https://git.io/JDfIx",
      " Back"
    }, {
      " <Pick a Song>",
      " POC PROCEDURAL",
      " Song 1",
      " Song 2",
      " Back",
    }
  };

  bool update() {
    bool c = updateCursor();
    bool m = updateMenu();
    if (m) {
      sectionIndex = 1;
      cursorRow = 1;
      // TODO somehow remember last index
    }
    return c || m;
  }

  bool updateCursor() {
    int dir = joystick.detectMoveY();
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

  const int blockValue = 21;

  void contrastMenu() {
    
    const int maxBlockCount = 12;
    const int units = 255 / maxBlockCount;
    int blockCount = settings.contrast / 21;

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
      int dir = joystick.detectMoveX();
      if (dir == 1 && blockCount < maxBlockCount) {
        lcd.setCursor(blockCount + 2, 0);
        lcd.write(byte(BLOCK));
        blockCount += 1;
      } else if (dir == -1 && blockCount > 0) {
        lcd.setCursor(blockCount + 1, 0);
        lcd.print(" ");
        blockCount -= 1;
      }

      settings.contrast = blockCount * units;
      setContrast();
      
      int buttonState = joystick.getButton();
      if (buttonState == 1) {
        break;
      }
    }

    // TODO eeprom
    // TODO split this in mai multe in caz ca imi trebuie pe bucati
  }

  bool updateMenu() {
    int buttonState = joystick.getButton();
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
        if (sectionIndex == menuLengths[PLAY] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case SETTINGS:
        if (sectionIndex == CONTRAST) {
          contrastMenu();
        }
        if (sectionIndex == menuLengths[SETTINGS] - 1) {
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
      lcd.setCursor(displayWidth - 1, 1);
      lcd.write(byte(D_ARROW));
    }
    if (sectionIndex > 1 || (sectionIndex == 1 && cursorRow == 0)) {
      lcd.setCursor(displayWidth - 1, 0);
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
        lcd.setCursor(displayWidth - 1 + scrollCount, cursorRow);
        lcd.print(menuSections[currentMenu][sectionIndex][displayWidth - 2 + scrollCount]);
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
    analogWrite(v0, settings.contrast);
  }
};

#endif
