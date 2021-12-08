#ifndef MENU_H
#define MENU_H

#include <LiquidCrystal.h>
#include "Joystick.h"

// TODO: singleton
class Menu {
public:
  #define MAIN_MENU 0
  #define SETTINGS_MENU 1
  #define ABOUT_MENU 2

  Menu(): lcd(RS, enable, d4, d5, d6, d7), joystick() {
    sectionIndex = 1;
    currentMenu = MAIN_MENU;
    cursorRow = 1;
    pinMode(v0, OUTPUT);
    lcd.begin(16, 2);
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

  Joystick joystick;

  struct {
    int contrast = 120;
  } settings;
  
  const byte RS = 13, enable = 6, d4 = 5, d5 = 4,
             d6 = 3, d7 = 7, v0 = 9;

//  const byte arrow_up[8] {
//    B00000,
//    B10001,
//    B00000,
//    B00000,
//    B10001,
//    B01110,
//    B00000,
//  }

  LiquidCrystal lcd;

  const String title = " Kinda OSU!";
  
  const int menuLengths[3] = {5, 6, 5};
  const String menuSections[3][6] = { {
      title,
      " Start",
      " Highscore",
      " Settings",
      " About"
    }, {
      " Settings",
      " Enter name",
      " Difficulty",
      " Contrast",
      " Brightness",
      " Back"
    }, {
      " About",
      " Game Title: " + title,
      " Creator: Stefan R.",
      " Github link: <link>",
      " Back"
    }
  };

  bool update() {
    bool c = updateCursor();
    bool m = updateMenu();
    if (m) {
      sectionIndex = 1;
      cursorRow = 1;
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

  bool updateMenu() {
    int buttonState = joystick.getButton();
    if (buttonState != 1) {
      return false;
    }
    Serial.println("button press");

    switch (currentMenu) {
      case MAIN_MENU:
        if (sectionIndex == 3) {
          currentMenu = SETTINGS_MENU;
        } else if (sectionIndex == 4) {
          currentMenu = ABOUT_MENU; 
        }
        break;
      case SETTINGS_MENU:
        if (sectionIndex == menuLengths[SETTINGS_MENU] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case ABOUT_MENU:
        if (sectionIndex == menuLengths[ABOUT_MENU] - 1) {
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
    lcd.print("-" + menuSections[currentMenu][sectionIndex]);
  }

  unsigned long lastScrollTime;
  const int scrollInterval = 500;
  int scrollCount;
  
  void scrollLongLines() {
      unsigned long timeNow = millis();
      if (scrollCount == 0 && timeNow - lastScrollTime < scrollInterval * 10) {
        return;
      } else if (scrollCount != 0 && timeNow - lastScrollTime < scrollInterval) {
        return;
      }
     
      if (menuSections[currentMenu][sectionIndex].length() > 14) {
        lcd.scrollDisplayLeft();
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
      if (scrollCount == menuSections[currentMenu][sectionIndex].length() + 1) {
        scrollCount = 0;
        showMenuSections();
      }
  }

  void setContrast() {
    analogWrite(v0, settings.contrast);
  }
};

#endif
