#ifndef CHARACTERS_H
#define CHARACTERS_H

#define BLOCK 0
byte block[8] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

#define R_ARROW 1
byte rightArrow[8] = {
  B00000,
  B00100,
  B11110,
  B11111,
  B11110,
  B00100,
  B00000,
};

#define U_ARROW 2
byte upArrow[8] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B01110,
  B01110,
  B01110,
};

#define D_ARROW 3
byte downArrow[8] = {
  B01110,
  B01110,
  B01110,
  B11111,
  B01110,
  B00100,
  B00000,
};

#endif
