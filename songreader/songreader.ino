#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>

#include "Defines.h"

const String melodyFileNames[MELODY_COUNT] = {
  "songs/HARRYP"
};

byte currentSongIndex = -1,
    windowLength = -1;

/*
 * used to handle the graceful quit
 */
bool running = true;

File melodyFile;

void initSDCard() {
  if (!SD.begin(SD_CARD_CS_PIN)) {
    Serial.println("SD card initialization failed!");
    while (true);
  }
}

void receiveHandler(int numBytes) {
  if (numBytes == NUM_BYTES_FROM_MASTER) {
    /*
     * Receive 1 byte - the index of the song to
     * be transmitted. The corresponding file 
     * whill be opened.
     */
    currentSongIndex = Wire.read();
    melodyFile = SD.open(melodyFileNames[currentSongIndex], FILE_READ);
    if (!melodyFile) {
      Serial.println("Could not open file");
      while (true);
    }
  } else {
    /*
     * Receive 1 byte equal to SHUT_DOWN 
     * & anything else => main loop stops, 
     * slave becomes unresponsive. used to quit
     */
    int value = Wire.read();
    if (value = SHUT_DOWN) {
      running = false;
    }
  }
}

int readIntFromMelodyFile() {
  /*
   * Read caracter by character and reconstruct
   * the next signed integer. use a bitwise optimisation
   */
  int nr = 0;
  char c = melodyFile.read();
  bool negative = false;

  /*
   * skip whiteline characters
   */
  while (c == '\n' || c == ' ') {
    c = melodyFile.read();
  }

  /*
   * If after skipping whitespaces there is anything left
   * there surely is at least one row left, based on the 
   * file structure
   */
  if (!melodyFile.available()) {
    return END_OF_FILE;
  }

  if (c == '-') {
    negative = true;
    c = melodyFile.read();
  }
  
  while (c >= '0' && c <= '9') {
    /* 
     *  Bitwise optimisation
     *  nr << 3 + nr << 1 == nr * 8 + nr * 2
     *  == nr * 10 (but faster)
     */
    nr = (nr << 3) + (nr << 1) + (int)(c - '0');
    c = melodyFile.read();
  }

  if (negative) {
    nr *= -1;
  }
  
  return nr;
}

void wireWriteInt(int val) {
  /*
   * In order write more characters I have to use arrays
   * I consider that an <int> is an array of characters in memory
   * and send it with by casting it to a (char*)
   */
  Wire.write((char*)&val, (int)sizeof(val));
}

void melodyPartsRequestHandler() {
  for (int i = 0; i < SENT_NOTES_ON_WINDOW; ++i) {
    int note = readIntFromMelodyFile();
    if (note == -1) {
      wireWriteInt(END_OF_FILE);
      melodyFile.close();
      return;
    }
    int duration = readIntFromMelodyFile();

    wireWriteInt(note);
    wireWriteInt(duration);
  }
  wireWriteInt(END_OF_WINDOW);
}

int main () {
  init ();  // initialize timers
  Serial.begin (9600);
  Serial.flush (); // let serial printing finish

  initSDCard();
  Wire.begin(SLAVE_NUMBER);
  Wire.onReceive(receiveHandler);
  Wire.onRequest(melodyPartsRequestHandler);

  while (running);
}
