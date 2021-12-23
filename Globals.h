#ifndef GLOBALS_H
#define GLOBALS_H

/*
 * PINS
 */

/* BUTTON GROUP */
const int buttonGroupButton1Pin = 3,
          buttonGroupButton2Pin = 6,
          buttonGroupButton3Pin = A3,
          buttonGroupButton4Pin = 13;

/* MATRIX */
const int matrixDinPin = 12,
          matrixClockPin = 11,
          matrixLoadPin = 10;

/* JOYSTICK */
const int joystickXPin = A1,
          joystickYPin = A0,
          joystickSWPin = 2;

/* LCD */
const int lcdV0Pin = 9,
          lcdDSPin = 4,
          lcdClockPin = 7,
          lcdLatchPin = 8;

const int lcdShiftRegisterRSPin = 1,
          lcdShiftRegisterEPin = 3,
          lcdShiftRegisterD4Pin = 4,
          lcdShiftRegisterD5Pin = 5,
          lcdShiftRegisterD6Pin = 6,
          lcdShiftRegisterD7Pin = 7;

/* SPEAKER */

const int speakerPin = 5;

/*
 * OTHER CONSTANTS & DEFINES
 */

#define SLAVE_NUMBER 7

/* SONGS */
#define HARRY_POTTER 0

/* GAME CONSTANTS */
#define MELODY_REQUEST_COUNT 7
// each melody part consists of note & duration => *2
// note & duration are ints => *(sizeof(int))
// I expect and ending value of -1 / -2 => +sizeof(int)
#define MELODY_BYTES_TO_RECEIVE (MELODY_REQUEST_COUNT * 2 * sizeof(int) + sizeof(int))
#define MELODY_BUFFER_LENGTH (MELODY_REQUEST_COUNT * 2)
#define MELODY_SECTION_END -2
#define MELODY_END -1

#define TEMPO 120
#define WHOLE_NOTE_DURATION (60000 / TEMPO) * 4
#define WHOLE_NOTE_BAR_COUNT 32
#define MELODY_BAR_DURATION (WHOLE_NOTE_DURATION / WHOLE_NOTE_BAR_COUNT)

#endif
