#ifndef MASTER_CLASS_H
#define MASTER_CLASS_H

#include <LiquidCrystal_74HC595.h>
#include "Joystick.h"
#include "Characters.h"
#include "Game.h"
#include "Globals.h"
#include <EEPROM.h>

class MasterClass {
public:

  MasterClass(): lcd(lcdDSPin, lcdClockPin, lcdLatchPin,
          lcdShiftRegisterRSPin, lcdShiftRegisterEPin,
          lcdShiftRegisterD4Pin, lcdShiftRegisterD5Pin,
          lcdShiftRegisterD6Pin, lcdShiftRegisterD7Pin) {
    joystick = Joystick::getInstance();
    sectionIndex = 1;
    currentMenu = MAIN_MENU;
    cursorRow = 1;
    
    // lcd related
    pinMode(lcdV0Pin, OUTPUT);

    /* 
     *  lcd.clear() before begin makes all the difference
     *  in terms of clean, 0 noise, startup
     */
    lcd.clear();
    lcd.begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lcd.noCursor();
    lcd.noBlink();

    createBlock(lcd);
    createRArrow(lcd);
    createLArrow(lcd);
    createUArrow(lcd);
    createDArrow(lcd);
    
    // get settings and init    
    loadAllFromStorage();
    // used to update highscore menu
    updateHighscores(0);
    
    game = new Game(settings.matrixBrightness);
    
    setContrast();
    setDifficulty();
    
    showStartMessage();
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

  byte sectionIndex, currentMenu, cursorRow;

  Game *game;
  Joystick *joystick;
  
  // settings are saved in and loaded from eeprom
  struct {
    int contrast; // 0 - 12
    int matrixBrightness; // 0 - 12
    int difficulty; // 0 - 12
    char playerName[PLAYER_NAME_LENGTH + 1];
  } settings;
      
  LiquidCrystal_74HC595 lcd;

  const byte menuLengths[SECTION_COUNT] = {5, 7, 5, 7, 5};
  String menuSection[MAX_SUBSECTIONS_COUNT];

  /*
   * this function exists as a means of saving memory
   * I save memory by allocating only for a section
   * in detriment of negligeble time loss for loading it
   * every time I need it
   */
  void loadMenuSection(int which) {
    if (which == MAIN_MENU) {
      menuSection[0] = "<Kinda OSU!>",
      menuSection[1] = "Let's OSU!",
      menuSection[2] = "Highscore",
      menuSection[3] = "Settings",
      menuSection[4] = "About";
    } else if (which == PLAY_MENU) {
      menuSection[0] = "<Pick Mode>",
      menuSection[1] = "Harry Potter",
      menuSection[2] = "Merry XMAS",
      menuSection[3] = "Pink Panther",
      menuSection[4] = "Star Wars",
      menuSection[5] = "Survival",
      menuSection[6] = "Back";
    } else if (which == HIGHSCORE_MENU) {
      menuSection[0] = "<Highscores>",
      loadHighscoresInMenuSection(),
      menuSection[4] = "Back";
    } else if (which == SETTINGS_MENU) {
      menuSection[0] = "<Settings>",
      menuSection[1] = "Enter name",
      menuSection[2] = "Contrast",
      menuSection[3] = "Mat Brightnes",
      menuSection[4] = "Difficulty",
      menuSection[5] = "Reset Hiscore",
      menuSection[6] = "Back";
    } else if (which == ABOUT_MENU) {
      menuSection[0] = "<About>",
      menuSection[1] = "Title: Kinda OSU!",
      menuSection[2] = "By: Stefan Radu",
      menuSection[3] = "Github: https://git.io/JDfIx",
      menuSection[4] = "Back";
    }
  }

  bool update() {
    bool c = updateCursor();
    bool m = switchMenu();
    if (m) {
      sectionIndex = 1;
      cursorRow = 1;
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
    printIndentedMessage(0, "Wellcome ");
    delay(500);
    lcd.print("and");
    delay(1000);
    printIndentedMessage(1, " Let's play");
    delay(1500);
    lcd.clear();
    delay(1000);
    printIndentedMessage(0, " OSU! Mania");
    delay(1500);
    printIndentedMessage(1, "  Kinda :P");
    delay(2000);
    lcd.clear();
    delay(1000);
  }

  void printIndentedMessage(byte row, const char *message) {
    lcd.setCursor(2, row);
    lcd.print(message);
  }

  /*
   * defines what happens for every section and
   * submenu when the joystick button is pressed
   * takes care of the sound played when the menu is changed
   */
  bool switchMenu() {
    int buttonState = joystick->getButton();
    if (buttonState != 1) {
      return false;
    }

    switch (currentMenu) {
      case MAIN_MENU:
        currentMenu = sectionIndex;
        break;
      case SETTINGS_MENU:
        if (sectionIndex == ENTER_NAME) {
          selectNameMenu();
        } else if (sectionIndex == CONTRAST) {
          sliderMenu(settings.contrast, MAX_CONTRAST_BLOCK_COUNT, &setContrast);
        } else if (sectionIndex == MAT_BRIGHTNESS) {
          game->setMatrix(B00111100);
          sliderMenu(settings.matrixBrightness, MAX_MAT_BRIGHTNESS_BLOCK_COUNT,
              &setMatrixBrightness);
          game->setMatrix(B00000000);
        } else if (sectionIndex == DIFFICULTY) {
          sliderMenu(settings.difficulty, MAX_DIFFICULTY_BLOCK_COUNT, &setDifficulty);
        } else if (sectionIndex == RESET_HIGHSCORE) {
          resetHighscores();
          currentMenu = MAIN_MENU;
        } else if (sectionIndex == menuLengths[SETTINGS_MENU] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case ABOUT_MENU:
        if (sectionIndex == menuLengths[ABOUT_MENU] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case HIGHSCORE_MENU:
        if (sectionIndex == menuLengths[HIGHSCORE_MENU] - 1) {
          currentMenu = MAIN_MENU;
        }
        break;
      case PLAY_MENU:
        if (sectionIndex == menuLengths[PLAY_MENU] - 1) {
          currentMenu = MAIN_MENU;
          break;
        }
        int score;
        if (sectionIndex == SURVIVAL) {
          score = game->playSurvival();
        } else {
          score = game->playSong(sectionIndex - 1, settings.playerName);
        }
        updateHighscores(score);
        currentMenu = PLAY_MENU;
        break;
    }

    if (currentMenu == MAIN_MENU) {
      tone(speakerPin, NOTE_D5, CLICK_TONE_DURATION);
      tone(speakerPin, NOTE_G4, CLICK_TONE_DURATION);
    } else {
      tone(speakerPin, NOTE_G4, CLICK_TONE_DURATION);
      tone(speakerPin, NOTE_D5, CLICK_TONE_DURATION);
    }
    
    return true;
  }

  /*
   * takes care of the pagination. 
   * if on the top row, only show down arrow
   * if on the botton row, only show up arrow
   * if a move on the Y axis does not cause a page
   *  change, then ONLY the arrow will move
   * else the arrow will stay in place and the page
   *  will change accordingly
   */
  void showMenuSections() {
    loadMenuSection(currentMenu);
    
    lcd.clear();
    int offset = 1 - cursorRow * 2; // mate
    lcd.setCursor(0, 1 - cursorRow);
    lcd.print("  " + menuSection[sectionIndex + offset]);
    lcd.setCursor(0, cursorRow);
    lcd.write(byte(R_ARROW));
    lcd.print(" ");
    lcd.print(menuSection[sectionIndex]);

    if (sectionIndex < menuLengths[currentMenu] - 1) {
      lcd.setCursor(DISPLAY_WIDTH - 1, 1);
      lcd.write(byte(D_ARROW));
    }
    if (sectionIndex > 1 || (sectionIndex == 1 && cursorRow == 0)) {
      lcd.setCursor(DISPLAY_WIDTH - 1, 0);
      lcd.write(byte(U_ARROW));
    }
  }

  /*
   * display a slider menu with a custom number of blocks
   * check joystick on X axis to increment / decrement the slider
   * use function passed as parameter to update the value modified
   * intended to be used in settings
   */
  void sliderMenu(int &activeBlockCount, const int maxBlockCount,
       void (MasterClass::*updateSettings)()) {

    lcd.clear();
    /* 4 comes from " -" & "+ " strings at each end */
    int padding = (DISPLAY_WIDTH - 4 - maxBlockCount) / 2;
    lcd.setCursor(padding, 0);
    lcd.print("- ");
    for (int i = 0; i < activeBlockCount; ++i) {
      lcd.write(byte(BLOCK));
    }
    for (int i = activeBlockCount; i < maxBlockCount; ++i) {
      lcd.print(" ");
    }
    lcd.print(" +");
    lcd.setCursor(1, 1);
    lcd.print("Press to save!");
  
    while (true) {
      int dir = joystick->detectMoveX();
      if (dir == 1 && activeBlockCount < maxBlockCount) {
        lcd.setCursor(padding + 2 + activeBlockCount, 0);
        lcd.write(byte(BLOCK));
        activeBlockCount += 1;
      } else if (dir == -1 && activeBlockCount > 1) {
        lcd.setCursor(padding + 1 + activeBlockCount, 0);
        lcd.print(" ");
        activeBlockCount -= 1;
      }
  
      (this->*updateSettings)();
      
      if (joystick->getButton()) {
        break;
      }
    }
    saveSettingsInStorage();
  }

  /*
   * cycle through positions with joyLeft / joyRight
   * cycle through letters at position with joyUp / joyDown
   * press to save
   * I made sure that you can cycle through small letters ->
   *  big letters -> space -> small letters by handling
   *  modifications at the edges in a cusom manner
   */
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
  int scrollCount;

  /*
   * wait SCROLL_ENABLE_TIMEOUT ms then start scolling
   * then, every SCROLL_INTERVAL ms scroll display to the left
   * to keep the line not selected in place print it repeatedly 
   *  with incresing padding
   *  override unwanted characters caused by scrolling by printing
   *  the correct carracter at an offset based on the scroll steps
   */
  void scrollLongLines() {
    unsigned long timeNow = millis();
    if (scrollCount == 0 && timeNow - lastScrollTime < SCROLL_ENABLE_TIMEOUT ) {
      return;
    } else if (scrollCount != 0 && timeNow - lastScrollTime < SCROLL_INTERVAL) {
      return;
    }

    loadMenuSection(currentMenu);
    int limit = menuSection[sectionIndex].length() - DISPLAY_PADDED_WIDTH;
    if (scrollCount == limit && timeNow - lastScrollTime < SCROLL_ENABLE_TIMEOUT ) {
      return;
    } else if (scrollCount == limit) {
      scrollCount = 0;
      lastScrollTime = millis();
      showMenuSections();
      return;
    }
   
    if (menuSection[sectionIndex].length() > DISPLAY_PADDED_WIDTH) {
      lcd.scrollDisplayLeft();
      lcd.setCursor(DISPLAY_WIDTH - 1 + scrollCount, cursorRow);
      lcd.print(menuSection[sectionIndex][DISPLAY_WIDTH - 3 + scrollCount]);
      int offset = 1 - cursorRow * 2;
      lcd.setCursor(0, 1 - cursorRow);
      String padding = "   ";
      for (int i = 0; i < scrollCount; ++ i) {
        padding += " ";
      }
      lcd.setCursor(0, 1 - cursorRow);
      lcd.print(padding + menuSection[sectionIndex + offset]);
    }
      
    lastScrollTime = timeNow;
    scrollCount += 1;
  }

/*
 * 
 * ================= Highscore logic =================
 * 
 */
 
  int highscores[HIGHSCORE_COUNT] = {0, 0, 0};
  char highscoreNames[HIGHSCORE_COUNT][PLAYER_NAME_LENGTH + 1];

  void loadHighscoresInMenuSection() {
    // update what is shown in the highscore menu
    for (int i = 0; i < 3; ++ i) {
      menuSection[i + 1] = String(highscoreNames[i]);
      menuSection[i + 1] += ": ";
      menuSection[i + 1] += String(highscores[i]);
    }
  }

  /*
   * insert the new highscore in it's corresponding place
   *  by comparing it to all the rest by hand
   */
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
    saveHighscoresInStorage();
  }

  /*
   * overrides everything with 0 and "Noname"
   */
  void resetHighscores() {
    highscores[2] = 0;
    highscores[1] = 0;
    highscores[0] = 0;
    // names corresponding to the highscores
    strcpy(highscoreNames[2], NO_NAME);
    strcpy(highscoreNames[1], NO_NAME);
    strcpy(highscoreNames[0], NO_NAME);
    saveHighscoresInStorage();
  }

/*
 * 
 * ================= Settings and persistent storage =================
 * 
 */

  void setContrast() {
    int value = map(settings.contrast, MIN_SLIDER_BLOCK_COUNT,
        MAX_CONTRAST_BLOCK_COUNT, MIN_CONTRAST, MAX_CONTRAST);
    analogWrite(lcdV0Pin, value);
  }

  void setMatrixBrightness() {
    int value = map(settings.matrixBrightness, MIN_SLIDER_BLOCK_COUNT,
        MAX_MAT_BRIGHTNESS_BLOCK_COUNT, MIN_MAT_BRIGHTNESS, MAX_MAT_BRIGHTNESS);
    game->updateBrightness(value);
  }

  void setDifficulty() {
    int value = map(settings.difficulty, MIN_SLIDER_BLOCK_COUNT,
        MAX_DIFFICULTY_BLOCK_COUNT, MIN_DIFFICULTY, MAX_DIFFICULTY);
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
