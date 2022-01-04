#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <LedControl.h>
#include "Globals.h"

class Symbols {
public:
  
  void smiley(bool won) {
    byte symbol[MATRIX_HEIGHT] {
      B00000000,
      B01100110,
      B01100110,
      B01100110,
      B00000000,
      B01000010,
      B00111100,
      B00000000
    };

    if (!won) {
      // make it sad
      byte aux = symbol[5];
      symbol[5] = symbol[6];
      symbol[6] = aux;
    }

    displaySymbol(symbol);
  }

  void osu() {
    byte symbol[MATRIX_HEIGHT] {
      B00111100,
      B00100000,
      B11100101,
      B10110101,
      B10101101,
      B11100111,
      B00000100,
      B00111100,
    };

    displaySymbol(symbol);
  }

  void settings() {
    byte symbol[MATRIX_HEIGHT] {
      B00000000,
      B00010100,
      B01011000,
      B00111110,
      B01111100,
      B00011010,
      B00101000,
      B00000000
    };

    displaySymbol(symbol);
  }

  void play() {
    byte symbol[MATRIX_HEIGHT] {
      B00011000,
      B00011100,
      B00010110,
      B00010010,
      B00010010,
      B00110100,
      B01110000,
      B01110000,
    };

    displaySymbol(symbol);
  }

  void highscore() {
    byte symbol[MATRIX_HEIGHT] {
      B01111110,
      B01111110,
      B01111110,
      B01111110,
      B00111100,
      B00011000,
      B00011000,
      B01111110
    };
 
    displaySymbol(symbol);
  }

  void about() {
    byte symbol[MATRIX_HEIGHT] {
      B00111000,
      B00011000,
      B00000000,
      B00111000,
      B00011000,
      B00011000,
      B00011000,
      B00011100,
    };
 
    displaySymbol(symbol);
  }

private:

  void displaySymbol(byte symbol[]) {
    for (int i = 0; i < MATRIX_HEIGHT; ++i) {
      lc.setRow(0, i, symbol[i]);
      delay(SHOW_SYMBOL_ANIMATION_DELAY);
    }
  }
};

#endif
