#ifndef CHARACTERS_H
#define CHARACTERS_H

#define BLOCK 0

/*
 * I don't store byte arrays for the carractes
 * in order to save memory. they are created and
 * forgotten
 */

void createBlock(LiquidCrystal_74HC595 &lcd) {
  byte block[8] = {
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
    B11111,
  };
  lcd.createChar(BLOCK, block);
}

#define R_ARROW 1
void createRArrow(LiquidCrystal_74HC595 &lcd) {
  byte rightArrow[8] = {
    B00000,
    B00100,
    B11110,
    B11111,
    B11110,
    B00100,
    B00000,
  };
  lcd.createChar(R_ARROW, rightArrow);
}

#define L_ARROW 2
void createLArrow(LiquidCrystal_74HC595 &lcd) {
  byte leftArrow[8] = {
    B00000,
    B00100,
    B01111,
    B11111,
    B01111,
    B00100,
    B00000,
  };
  lcd.createChar(L_ARROW, leftArrow);
}

#define U_ARROW 3
void createUArrow(LiquidCrystal_74HC595 &lcd) {
  byte upArrow[8] = {
    B00000,
    B00100,
    B01110,
    B11111,
    B01110,
    B01110,
    B01110,
  };
  lcd.createChar(U_ARROW, upArrow);
}

#define D_ARROW 4
void createDArrow(LiquidCrystal_74HC595 &lcd) {
  byte downArrow[8] = {
    B01110,
    B01110,
    B01110,
    B11111,
    B01110,
    B00100,
    B00000,
  };
  lcd.createChar(D_ARROW, downArrow);
}

#endif
